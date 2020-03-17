#include "ndevice_spi.h"
#include "nrf_delay.h"
#include "nrf_gpio.h"

static stSPIpsel* pSPIpsel_g = ((stSPIpsel*)ND_SPI1_PSEL);
static stSPIData* pSPIdata_g = ((stSPIData*)ND_SPI1_DATA);
static stSPIinterrupt* pSPIinterrupt_g = ((stSPIinterrupt*)ND_SPI1_INTERRUPT);

fnSPIReceiveHandler cbSPIReceiveHandler_g;

static uint32_t pin_scs_g = 0;
static uint8_t IsSPIBusy_g = 0;

/*
*/
uint8_t IsSPIBusy(void)
{
  return IsSPIBusy_g;
}

/*
* @brief : clear events
*/
static void nd_spim_clear_events(void)
{
  //Interrupt clear&enabled
  ND_SPI1_EVNT_STOPPED = 0;
  ND_SPI1_EVNT_ENDRX = 0;
  ND_SPI1_EVNT_END = 0;
  ND_SPI1_EVNT_ENDTX = 0;
  ND_SPI1_EVNT_STARTED = 0;
}
/*
* @brief : Initialize SPI
* @ret : FALSE->Failed to connect, TRUE->Succeed to conned
*/
bool nd_spi_init(uint32_t PinSCK, uint32_t PinSDI, uint32_t PinSDO, uint32_t PinSCS)
{
  bool ret = true;
  
  ND_SPI1_TASKSTOP = 1;
  nrf_delay_ms(10);
  ND_SPI1_ENABLE = 0UL;
  nrf_delay_ms(10);

  nd_spim_clear_events();

  //ND_SPI1_CONFIG = ;              //Using defulat

  pin_scs_g = PinSCS;
  nrf_gpio_cfg_output(pin_scs_g);
  nrf_gpio_pin_set(pin_scs_g);

  pSPIpsel_g->miso = PinSDI;
  pSPIpsel_g->mosi = PinSDO;
  pSPIpsel_g->sck = PinSCK;

  ND_SPI1_FREQ = ND_SPI_FREQ_M8;               //8Mhaz clock
  
  ND_SPI1_ENABLE = ND_SPI1_EN_ENABLE;

  //Interrupt enabled
  pSPIinterrupt_g->set = ND_SPI_INT_END;
  
  NRFX_IRQ_PRIORITY_SET(SPIM1_SPIS1_TWIM1_TWIS1_SPI1_TWI1_IRQn, 7);
  NRFX_IRQ_ENABLE(SPIM1_SPIS1_TWIM1_TWIS1_SPI1_TWI1_IRQn);  
  
  if(pSPIpsel_g->miso & 0x80000000UL) ret = false;
  if(pSPIpsel_g->mosi & 0x80000000UL) ret = false;
  
  return ret;
}

/*
* @brief : send data
*/
void spim_senddata(uint8_t* pBuf, uint32_t len)
{
  if(pin_scs_g == 0)
  {
    nrf_gpio_pin_set(pin_scs_g);
    nrf_delay_us(10);
  }

  nrf_gpio_pin_clear(pin_scs_g);

  pSPIdata_g->txd_maxcnt = len;
  pSPIdata_g->txd_ptr = pBuf;

  pSPIdata_g->rxd_maxcnt = 0;

  cbSPIReceiveHandler_g = 0;

  nd_spim_clear_events();

  IsSPIBusy_g = 1;
  ND_SPI1_START = 1;
}

/*
* @brief : receive data, using callback function
*/
void spim_repeateddata(uint8_t* pTxBuf, uint32_t TxLen, uint8_t* pRxBuf, uint32_t RxLen, fnSPIReceiveHandler handler)
{
  if(pin_scs_g == 0)
  {
    nrf_gpio_pin_set(pin_scs_g);
    nrf_delay_us(10);
  }

  nrf_gpio_pin_clear(pin_scs_g);

  pSPIdata_g->txd_maxcnt = TxLen;
  pSPIdata_g->txd_ptr = pTxBuf;

  pSPIdata_g->rxd_maxcnt = RxLen;
  pSPIdata_g->rxd_ptr = pRxBuf;

  cbSPIReceiveHandler_g = handler;

  nd_spim_clear_events();

  IsSPIBusy_g = 1;
  ND_SPI1_START = 1;
}

/*
* @brief : Interrupt service routine
*/
void SPIM1_SPIS1_TWIM1_TWIS1_SPI1_TWI1_IRQHandler(void)
{
  if(ND_SPI1_EVNT_END)
  {
    ND_SPI1_EVNT_END = 0;
    
    IsSPIBusy_g = 0;

    nrf_gpio_pin_set(pin_scs_g);
    if(cbSPIReceiveHandler_g != 0) cbSPIReceiveHandler_g();
    
    //SEGGER_RTT_printf(0, "spi cleared");
  }
}
