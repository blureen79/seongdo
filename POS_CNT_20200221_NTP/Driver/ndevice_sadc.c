#include "nrf52.h"
#include "ndevice_sadc.h"
#include "ndevice_util.h"
#include "nrf_drv_common.h"

stSADCconfig* pADCconfig_g = ((stSADCconfig*)ND_SADC_CONFIG);
stSADCinterrupt* pADCinterrupt_g = ((stSADCinterrupt*)ND_SADC_INTERRUPT);
stSADCoperate* pADCoperate_g = ((stSADCoperate*)ND_SADC_OPERATE);
stSADCchannel* pADCchannel_g = ((stSADCchannel*)ND_SADC_SET_CH);
stSADCresult* pADCresult = ((stSADCresult*)ND_SADC_RESULT);
stADCcontrol ADCctrl_g;

uint32_t u32ADCResult_g[8];

/*
* @brief : Initialize ADC
*/
void adc_init(void)
{
  ND_SADC_ENABLE = 0;

  //For Battery status
  pADCchannel_g->pselp = ND_SADC_IN_Analog2;
  //pADCchannel_g->pselp = ND_SADC_IN_VDD;
  pADCchannel_g->pseln = ND_SADC_IN_NC;
  //pADCchannel_g->config = ND_SADC_RESP_Bypass | ND_SADC_RESN_Bypass | ND_SADC_GAIN1_6 | ND_SADC_TACQ_10us | ND_SADC_REF_Internal | ND_SADC_MODE_SE;
  pADCchannel_g->config = ND_SADC_RESP_Bypass | ND_SADC_RESN_Bypass | ND_SADC_GAIN1_4 | ND_SADC_TACQ_10us | ND_SADC_REF_VDD1_4 | ND_SADC_MODE_SE;

  pADCconfig_g->resolution = ND_SADC_RES14BIT;
  pADCconfig_g->oversample = ND_SADC_Over_2x;
  //pADCconfig_g->samplerate = ND_SADC_SAMPLE_CC | 5;
  pADCconfig_g->samplerate = ND_SADC_SAMPLE_Task;

  //Set scan mode

  pADCresult->pResult = (uint32_t)u32ADCResult_g;
  pADCresult->maxcnt = 1;
  //set max count

  //Interrupt clear&enabled
  ND_SADC_EVNT_END = 0;
  ND_SADC_EVNT_DONE = 0;
  ND_SADC_EVNT_RESULTDONE = 0;

  pADCinterrupt_g->inten = ND_SADC_INT_RESULTDONE | ND_SADC_INT_END;

  //nrf_drv_common_irq_enable(SAADC_IRQn, 7);

  NRFX_IRQ_PRIORITY_SET(SAADC_IRQn, 7);
  NRFX_IRQ_ENABLE(SAADC_IRQn);

  ND_SADC_ENABLE = 1;
  
  ADCctrl_g.num_of_ch = 1;
}

/*
* @brief : Start ADC
*/
void adc_start(void)
{
  pADCoperate_g->start = 1;
  ADCctrl_g.done = false;
  
  adc_sample();
}

void adc_sample(void)
{
  pADCoperate_g->sample = 1;
}

/*
* @brief : Stop ADC
*/
void adc_stop(void)
{
  uint32_t ret;

  pADCoperate_g->stop = 1;

  ret = (*(volatile uint32_t*)(ND_SADC_BASE + 0x00000400UL));
  while(ret) {};
}

/*
* @brief : get ADC result
*/
uint16_t adc_getdata(void)
{
  return (uint16_t)u32ADCResult_g[0];
}

/*
* @brief : get ADC result
*/
void get_adc_array(uint32_t *pBuff)
{
  *pBuff = u32ADCResult_g[0];
}


/*
* @brief : ADC busy check
*/
bool adc_IsBusy(void)
{
  bool ret;

#if 1
  if((*(volatile uint32_t*)(ND_SADC_BASE + 0x00000400UL)) != 0x00000000) ret = true;
  else ret = false;
#else
  //ret = ADCctrl_g.done ? 0:1;
  ret = ADCctrl_g.done;
  
  if(ret != 0) return false;         //it means ready to get next adc
  else return true;
#endif

  return ret;
}

/*
* @brief : ISR for SAADC
*/
void SAADC_IRQHandler(void)
{
  if(ND_SADC_EVNT_END)
  {
    ND_SADC_EVNT_END = 0;
    
    ADCctrl_g.done = true;
  }
  
  if(ND_SADC_EVNT_DONE)
  {
    ND_SADC_EVNT_DONE = 0;
  }
  
  if(ND_SADC_EVNT_RESULTDONE)
  {
    ND_SADC_EVNT_RESULTDONE = 0;
    
    if(--ADCctrl_g.num_of_ch)
    {
      adc_sample();
    }
  }
}
