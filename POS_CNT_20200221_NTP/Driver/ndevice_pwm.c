#include "sdk_config.h"
#include "custom_board.h"
#include "ndevice_pwm.h"

static stPWMoperate* pPWMoperate_g = ((stPWMoperate*)ND_PWM_BASE);

/*
* @brief : Initialize pwm
*/
void nd_pwm_init(nrf_drv_pwm_t* m_pwm, nrf_drv_pwm_config_t *config, nrf_pwm_sequence_t* seq)
{
  m_pwm->p_registers->PSEL.OUT[0] =   (config->output_pins[0] << PWM_PSEL_OUT_PIN_Pos)
                                                | (PWM_PSEL_OUT_CONNECT_Connected << PWM_PSEL_OUT_CONNECT_Pos);
  m_pwm->p_registers->PSEL.OUT[1] =   (config->output_pins[1] << PWM_PSEL_OUT_PIN_Pos)
                                                | (PWM_PSEL_OUT_CONNECT_Connected << PWM_PSEL_OUT_CONNECT_Pos);
  m_pwm->p_registers->PSEL.OUT[2] =   (config->output_pins[2] << PWM_PSEL_OUT_PIN_Pos)
                                                | (PWM_PSEL_OUT_CONNECT_Disconnected << PWM_PSEL_OUT_CONNECT_Pos);
  m_pwm->p_registers->PSEL.OUT[3] =   (config->output_pins[3] << PWM_PSEL_OUT_PIN_Pos)
                                                | (PWM_PSEL_OUT_CONNECT_Disconnected << PWM_PSEL_OUT_CONNECT_Pos);

  m_pwm->p_registers->ENABLE = (PWM_ENABLE_ENABLE_Enabled << PWM_ENABLE_ENABLE_Pos);
  m_pwm->p_registers->MODE = (PWM_MODE_UPDOWN_Up << PWM_MODE_UPDOWN_Pos);
  m_pwm->p_registers->PRESCALER = (PWM_PRESCALER_PRESCALER_DIV_4 << PWM_PRESCALER_PRESCALER_Pos);
  m_pwm->p_registers->COUNTERTOP = (10000 << PWM_COUNTERTOP_COUNTERTOP_Pos);
  m_pwm->p_registers->DECODER =   (PWM_DECODER_LOAD_Individual << PWM_DECODER_LOAD_Pos) | (PWM_DECODER_MODE_RefreshCount << PWM_DECODER_MODE_Pos);

  m_pwm->p_registers->SEQ[0].PTR = (uint32_t)seq->values.p_individual;
  m_pwm->p_registers->SEQ[0].CNT = ((4) << PWM_SEQ_CNT_CNT_Pos);
  m_pwm->p_registers->SEQ[0].REFRESH = 40;
  m_pwm->p_registers->SEQ[0].ENDDELAY = 0;
  m_pwm->p_registers->SEQ[1].PTR = (uint32_t)seq->values.p_individual;
  m_pwm->p_registers->SEQ[1].CNT = ((4) << PWM_SEQ_CNT_CNT_Pos);
  m_pwm->p_registers->SEQ[1].REFRESH = 40;
  m_pwm->p_registers->SEQ[1].ENDDELAY = 0;  
  
  nd_pwm_start(m_pwm);
}
/*
* @brief : stop pwm
*/
void nd_pwm_stop(nrf_drv_pwm_t* m_pwm)
{
  m_pwm->p_registers->SHORTS = PWM_SHORTS_LOOPSDONE_STOP_Msk;
}

/*
* @brief : set duty, repeat count
*/
void nd_pwm_duty(uint32_t duty_on, uint32_t duty_off, uint32_t repeat)
{
    //(void)nrf_drv_pwm_simple_playback(m_pwm, seq, 1, NRF_DRV_PWM_FLAG_LOOP);
}

/*
* @brief : start pwm out
*/
void nd_pwm_start(nrf_drv_pwm_t* m_pwm)
{
  m_pwm->p_registers->SHORTS = PWM_SHORTS_LOOPSDONE_SEQSTART0_Msk;
  m_pwm->p_registers->LOOP = (1 << PWM_LOOP_CNT_Pos);
  m_pwm->p_registers->TASKS_SEQSTART[0] = 1;
}

/*
* @brief : next step for pwm
*/
void nd_pwm_nextstep(void)
{
  pPWMoperate_g->nextstep = 1;
}
