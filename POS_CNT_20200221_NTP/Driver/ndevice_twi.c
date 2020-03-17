#include "ndevice_twi.h"

static stTWIconfig* pTWIconfig_g = ((stTWIconfig*)ND_TWI0_CONFIG);
static stTWIinterrupt* pTWIinterrupt_g = ((stTWIinterrupt*)ND_TWI0_INTERRUPT);
static stTWIoperate* pTWIoperate_g = ((stTWIoperate*)ND_TWI0_OPERATE);
static stTWIdata* pTWIdata_g = (stTWIdata*)ND_TWI0_DATA;

static uint32_t* pTWIaddress_g = (uint32_t*)ND_TWI0_ADDRESS;
static uint32_t* pTWIevent_txdsent = (uint32_t*)ND_TWI0_EVNT_TXDSENT;
stTWIbuffer pTWIbuffer_g;

typedef void (*fnReceiveHandler)(uint8_t*);

fnReceiveHandler cbReceiveHandler_g;

/*
* @brief : Initialize TWI
* @ret : FALSE->Failed to connect, TRUE->Succeed to conned
*/
bool nd_twi_init(uint32_t PinSCL, uint32_t PinSDA)
{
  bool ret = true;
  
  pTWIconfig_g->pselscl = PinSCL;
  pTWIconfig_g->pselsda = PinSDA;
  pTWIdata_g->freq = ND_TWI_FREQ_400K;

  pTWIconfig_g->enable = 1;

  //Interrupt enabled
  pTWIinterrupt_g->intenset = ND_TWI_INT_RXDREADY | ND_TWI_INT_TXDSENT | ND_TWI_INT_ERROR;
  
  if(pTWIconfig_g->pselscl & 0x80000000UL) ret = false;
  if(pTWIconfig_g->pselsda & 0x80000000UL) ret = false;
  
  return ret;
}

/*
* @brief : get received TWI data
*/
uint8_t twi_getdata(void)
{
  return (uint8_t)pTWIdata_g->rxd;
}

/*
* @brief : send data
*/
void twi_senddata(uint8_t addr, uint8_t* pBuf, uint8_t len)
{
  int i;
  
  *pTWIaddress_g = (uint32_t)addr;
  
  pTWIbuffer_g.len = len;
  pTWIbuffer_g.index = 0;
  
  for(i=0; i<len; ++i)
  {
    pTWIbuffer_g.pBuff[i] = pBuf[i];
  }
  
  pTWIdata_g->txd = pTWIbuffer_g.pBuff[pTWIbuffer_g.index++];
  pTWIoperate_g->startTx = 1;
}

/*
* @brief : process receive
*/
void twi_process_rxd(uint8_t* pBuf, uint8_t len, fnReceiveHandler handler)
{
  cbReceiveHandler_g = handler;  
}

/*
* @brief : check data is sent
*/
bool twi_IsSent(void)
{
  return *pTWIevent_txdsent;
}

/*
* @brief : Interrupt service routine
*/
void TWI_ISR(void)
{
  if(pTWIbuffer_g.index < pTWIbuffer_g.len)
  {
    pTWIdata_g->txd = (uint32_t)pTWIbuffer_g.pBuff[pTWIbuffer_g.index++];
  }
  else
  {
  }
  
  //call cbfunction
  //cbReceiveHandler_g();
  
  cbReceiveHandler_g(pTWIbuffer_g.pBuff);
}
