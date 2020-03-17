#include "ndevice_timer.h"

void nd_timer_init(void)
{
    //Enable  timer 
    NVIC_DisableIRQ(TIMER1_IRQn); // use "sd_nvic" prefix with softdevice
    
    nrf_timer_task_trigger(NRF_TIMER1, NRF_TIMER_TASK_STOP);
    nrf_timer_task_trigger(NRF_TIMER1, NRF_TIMER_TASK_CLEAR);
    
    nrf_timer_mode_set(NRF_TIMER1, NRF_TIMER_MODE_TIMER); 
    nrf_timer_bit_width_set(NRF_TIMER1, NRF_TIMER_BIT_WIDTH_32);
    nrf_timer_frequency_set(NRF_TIMER1, NRF_TIMER_FREQ_1MHz);
    nrf_timer_cc_write(NRF_TIMER1, NRF_TIMER_CC_CHANNEL0, 10000);     // 10ms
    nrf_timer_int_enable(NRF_TIMER1, NRF_TIMER_INT_COMPARE0_MASK);
    nrf_timer_shorts_enable(NRF_TIMER1, NRF_TIMER_SHORT_COMPARE0_CLEAR_MASK); // self running
    
    NVIC_SetPriority(TIMER1_IRQn, 7); // use "sd_nvic" prefix with softdevice
    NVIC_EnableIRQ(TIMER1_IRQn); // use "sd_nvic" prefix with softdevice    

    //nrf_timer_task_trigger(NRF_TIMER1, NRF_TIMER_TASK_START);
}

void TIMER1_IRQHandler(void)  // 10ms..
{
  if(NRF_TIMER1->EVENTS_COMPARE[0] == 1)
  {
    NRF_TIMER1->EVENTS_COMPARE[0] = 0;
  }
}
