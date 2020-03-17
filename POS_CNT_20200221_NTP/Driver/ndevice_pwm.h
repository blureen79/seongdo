/*
Author : Jaeyoung, LEE(NDevice.com)
Date : 01.15.2019
*/
#ifndef __NDEVICE_PWM
#define __NDEVICE_PWM

#include <stdbool.h>
#include <stdint.h>
#include <string.h>
#include "nrf.h"
#include "nrf_drv_common.h"
#include "nrf_drv_pwm.h"

typedef struct
{
  volatile uint32_t stop;                   //0x004
  volatile uint32_t seqstart0;              //0x008
  volatile uint32_t seqstart1;              //0x00C
  volatile uint32_t nextstep;               //0x010
} stPWMoperate;

typedef struct
{
  volatile uint32_t out[4];
} stPWMsel;

typedef struct
{
  volatile uint32_t ptr;
  volatile uint32_t cnt;
  volatile uint32_t refresh;
  volatile uint32_t enddelay;
} stPWMseq;

#define ND_PWM_BASE                (0x4001C000UL)                      //Base address of PWM0
#define ND_PWM_OPERATE             (ND_PWM_BASE + 0x00000004UL)        //Base address of PWM operation
#define ND_PWM_INTERRUPT           (ND_PWM_BASE + 0x00000300UL)        //Base address of PWM interrupt
#define ND_PWM_PSEL                (ND_PWM_BASE + 0x00000560UL)        //Base address of PWM psel
#define ND_PWM_SEQ                 (ND_PWM_BASE + 0x00000520UL)        //Base address of PWM seq

#define ND_PWM_SHORTS              (*(volatile uint32_t*)(ND_PWM_BASE + 0x00000200UL))         //Base address of PWM shortcut
#define ND_PWM_ENABLE              (*(volatile uint32_t*)(ND_PWM_BASE + 0x00000500UL))         //Base address of PWM enable
#define ND_PWM_MODE                (*(volatile uint32_t*)(ND_PWM_BASE + 0x00000504UL))         //Base address of PWM mode
#define ND_PWM_COUNTERTOP          (*(volatile uint32_t*)(ND_PWM_BASE + 0x00000508UL))         //Base address of PWM countertop
#define ND_PWM_PRESCALER           (*(volatile uint32_t*)(ND_PWM_BASE + 0x0000050CUL))         //Base address of PWM prescaler
#define ND_PWM_DECODER             (*(volatile uint32_t*)(ND_PWM_BASE + 0x00000510UL))         //Base address of PWM decoder
#define ND_PWM_LOOP                (*(volatile uint32_t*)(ND_PWM_BASE + 0x00000514UL))         //Base address of PWM loop

#define ND_PWM_DIV1                 (0x00000000UL)
#define ND_PWM_DIV16                (0x00000004UL)

extern void nd_pwm_init(nrf_drv_pwm_t* m_pwm, nrf_drv_pwm_config_t *config, nrf_pwm_sequence_t* seq);
extern void nd_pwm_start(nrf_drv_pwm_t* m_pwm);
extern void nd_pwm_stop(nrf_drv_pwm_t* m_pwm);
extern void nd_pwm_nextstep(void);
#endif
