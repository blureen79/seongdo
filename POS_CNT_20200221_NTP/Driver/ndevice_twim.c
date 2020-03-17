#include "ndevice_twim.h"
#include "ndevice_uart.h"
#include "ndevice_util.h"

static stTWIMconfig* pTWIconfig_g = ((stTWIMconfig*)ND_TWIM_CONFIG);
static stTWIMinterrupt* pTWIinterrupt_g = ((stTWIMinterrupt*)ND_TWIM_INTERRUPT);
static stTWIMoperate* pTWIoperate_g = ((stTWIMoperate*)ND_TWIM_OPERATE);
static stTWIMdata* pTWIdata_g = (stTWIMdata*)ND_TWIM_DATA;

static volatile uint32_t* pTWIaddress_g = (volatile uint32_t*)ND_TWIM_ADDRESS;

fnReceiveHandler cbReceiveHandler_g;
bool IsProcessing_g = false;

static void nd_twim_clear_events(void)
{
  //Interrupt clear&enabled
  ND_TWIM_EVNT_TXDSENT = 0;
  ND_TWIM_EVNT_ERROR = 0;
  ND_TWIM_EVNT_LASTRX = 0;
  ND_TWIM_EVNT_LASTTX = 0;
  ND_TWIM_EVNT_STOPPED = 0;
}

void nd_twim_gen_stop(void)
{
  pTWIoperate_g->stop = 1;
}

/*
* @brief : Initialize TWI
* @ret : FALSE->Failed to connect, TRUE->Succeed to conned
*/
bool nd_twim_init(uint32_t PinSCL, uint32_t PinSDA, stTWIMdata cfg_data)
{
  bool ret = true;

  pTWIoperate_g->stop = 1;
  nrf_delay_ms(10);
  pTWIconfig_g->enable = ND_TWIM_DISABLE;
  nrf_delay_ms(10);

  //NRF_GPIO->PIN_CNF[PinSCL] = (3 << 8) | (3 << 2); //configure pins with pullup, standard '0', disconnect '1'
  NRF_GPIO->PIN_CNF[PinSCL] = (3 << 8); //configure pins with no pullup, standard '0', disconnect '1'
  nrf_delay_us(5);
  //NRF_GPIO->PIN_CNF[PinSDA] = (6 << 8) | (3 << 2);
  NRF_GPIO->PIN_CNF[PinSDA] = (3 << 8) | (1 << 1);
  nrf_delay_us(5);

  pTWIconfig_g->pselscl = PinSCL;
  pTWIconfig_g->pselsda = PinSDA;

  //*(volatile uint32_t*)ND_TWIM_FREQUENCY = 0x02FF0000UL;
  //*(volatile uint32_t*)ND_TWIM_FREQUENCY = ND_TWIM_FREQ_100K;
  //*(volatile uint32_t*)ND_TWIM_FREQUENCY = ND_TWIM_FREQ_250K;
  *(volatile uint32_t*)ND_TWIM_FREQUENCY = ND_TWIM_FREQ_400K;

  //EasyDMA
  pTWIdata_g->rx_maxcount = cfg_data.rx_maxcount;
  pTWIdata_g->rx_list = cfg_data.rx_list;              //Use array list  
  pTWIdata_g->tx_maxcount = cfg_data.tx_maxcount;
  pTWIdata_g->tx_list = cfg_data.tx_list;              //Use array list

  IsProcessing_g = false;                   //To block overlapped starting

  //Interrupt clear&enabled
  nd_twim_clear_events();

  //Clear shortcuts
  ND_TWIM_SHORTS = 0;

  //pTWIinterrupt_g->intenable = ND_TWIM_INT_TXSTARTED | ND_TWIM_INT_LASTRX | ND_TWIM_INT_LASTTX | ND_TWIM_INT_ERROR | ND_TWIM_INT_STOPPED;
  pTWIinterrupt_g->intenable = ND_TWIM_INT_LASTRX | ND_TWIM_INT_LASTTX | ND_TWIM_INT_ERROR | ND_TWIM_INT_STOPPED;
  //pTWIinterrupt_g->intenable = ND_TWIM_INT_ERROR | ND_TWIM_INT_STOPPED;

  if(pTWIconfig_g->pselscl & 0x80000000UL) ret = false;
  if(pTWIconfig_g->pselsda & 0x80000000UL) ret = false;

  NRFX_IRQ_PRIORITY_SET(SPIM1_SPIS1_TWIM1_TWIS1_SPI1_TWI1_IRQn, 7);
  NRFX_IRQ_ENABLE(SPIM1_SPIS1_TWIM1_TWIS1_SPI1_TWI1_IRQn);
  //nrf_drv_common_irq_enable(SPIM1_SPIS1_TWIM1_TWIS1_SPI1_TWI1_IRQn, 7);

  pTWIconfig_g->enable = ND_TWIM_ENABLE;

  twim_clear_busy();
  
  return ret;
}

/*
@brief : twim error handler
*/
static void twim_err_handler(void)
{
  
}

/*
*/
void twim_clear_busy(void)
{
  IsProcessing_g = false;
}

/*
* @brief : send data
*/
bool twim_senddata(uint8_t addr, volatile uint8_t* pBuf, uint8_t len)
{
  bool ret = false;
  int i;
  
  if(IsProcessing_g == true) 
  {
    return ret; 
  }

  *pTWIaddress_g = (uint32_t)addr;

  pTWIdata_g->tx_maxcount = (uint32_t)len;
  pTWIdata_g->tx_pointer = pBuf;

  ND_TWIM_SHORTS = ND_TWIM_SHORT_LTSP;                                  //When reached to LASTTX and will goto STOP
  IsProcessing_g = true;
  
  nd_twim_clear_events();
  pTWIoperate_g->startTx = 1;

  for(i=0; i<30; ++i)
  {
    if(twim_IsBusy() == false) break;
    else nrf_delay_us(20);
    //else nrf_delay_ms(1);
  }

  ret = true;
  return ret;
}

/*
* @brief : receive data, using callback function
*/
bool twim_repeateddata(uint8_t addr, volatile uint8_t* pTxBuf, uint8_t TxLen, volatile uint8_t* pRxBuf, uint8_t RxLen, fnReceiveHandler handler)
{
  bool ret = false;
  int i;
  
  if(IsProcessing_g == true) 
  {
    return ret;  
  }

  *pTWIaddress_g = (uint32_t)addr;

  pTWIdata_g->tx_maxcount = (uint32_t)TxLen;
  pTWIdata_g->tx_pointer = pTxBuf;

  pTWIdata_g->rx_maxcount = (uint32_t)RxLen;
  pTWIdata_g->rx_pointer = pRxBuf;

  cbReceiveHandler_g = handler;

  ND_TWIM_SHORTS = ND_TWIM_SHORT_LTSR | ND_TWIM_SHORT_LRSP;            //Clear LASTTX and STARTRX

  IsProcessing_g = true;
  
  nd_twim_clear_events();
    
  pTWIoperate_g->startTx = 1;
  
#if 0
  while(twim_IsBusy() == false) {}
#else
  for(i=0; i<30; ++i)
  {
    if(twim_IsBusy() == false) break;
    else nrf_delay_us(20);
  }
#endif

  ret = true;
  
  return ret;
}

/*
* @brief : receive data, using callback function
*/
void twim_rcvdata(uint8_t addr, volatile uint8_t* pBuf, uint8_t len, fnReceiveHandler handler)
{
  if(IsProcessing_g == true) return; 
  *pTWIaddress_g = (uint32_t)addr;
  
  pTWIdata_g->rx_maxcount = len;
  pTWIdata_g->rx_pointer = pBuf;
  
  cbReceiveHandler_g = handler;
  
    //Shortcut enable, shortcut makes more safe
  ND_TWIM_SHORTS = ND_TWIM_SHORT_LRSP;            //When reached to LASTRX

  IsProcessing_g = true;
  nd_twim_clear_events();
  pTWIoperate_g->startRx = 1;
}

/*
* @brief : check TWIM is busy
*/
bool twim_IsBusy(void)
{
  return IsProcessing_g;
}

/*
* @brief : Interrupt service routine
Write 0 to clear event(14.6 Events on Page.60)
*/
//void SPIM0_SPIS0_TWIM0_TWIS0_SPI0_TWI0_IRQHandler(void)
void SPIM1_SPIS1_TWIM1_TWIS1_SPI1_TWI1_IRQHandler(void)
{
  if(ND_TWIM_EVNT_TXDSENT)
  {
    ND_TWIM_EVNT_TXDSENT = 0;
    //pTWIoperate_g->stop = 1;
  }

  if(ND_TWIM_EVNT_ERROR)
  {
    ND_TWIM_EVNT_ERROR = 0;
    pTWIoperate_g->stop = 1;
    IsProcessing_g = false;
    
    if(cbReceiveHandler_g)
    {
      cbReceiveHandler_g = (fnReceiveHandler)0x00UL;
    }
  }
  
  if(ND_TWIM_EVNT_STOPPED)
  {
    ND_TWIM_EVNT_STOPPED = 0;
    IsProcessing_g = false;
        
    if(cbReceiveHandler_g)
    {
      //uart_putc((uint8_t)pTWIdata_g->rx_amount);                  //Amout of received data
      
      cbReceiveHandler_g();
      cbReceiveHandler_g = (fnReceiveHandler)0x00UL;
    }
  }

  if(ND_TWIM_EVNT_LASTRX)
  {
    ND_TWIM_EVNT_LASTRX = 0;      //Cleared by shortcut
    //if reached to last RX or TX, then must set stop task
  }

  if(ND_TWIM_EVNT_LASTTX)
  {
    ND_TWIM_EVNT_LASTTX = 0;      //cleared by shortcut
    //if reached to last RX or TX, then must set stop task
  }
}
