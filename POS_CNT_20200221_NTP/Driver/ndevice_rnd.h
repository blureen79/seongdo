/*
Author : Jaeyoung, LEE(NDevice.com)
Date : 03.18.2017
*/
#ifndef __NDEVICE_RND
#define __NDEVICE_RND

#include <stdbool.h>
#include <stdint.h>
#include <string.h>
#include "nrf.h"
#include "nrf_drv_common.h"

typedef struct
{
  volatile uint32_t start;                  //0x000
  volatile uint32_t stop;                   //0x004
} stRNDoperate;

#define ND_RND_BASE                (0x4000D000UL)                      //Base address of TWIM0
#define ND_RND_OPERATE             (ND_RND_BASE)                       //Base address of TWI start/stop
#define ND_RND_INTERRUPT           (ND_RND_BASE + 0x00000304UL)         //Base address of TWI interrupt

#define ND_RND_SHORTS              (*(volatile uint32_t*)(ND_RND_BASE + 0x00000200UL))         //Base address of RNG shortcut
#define ND_RND_CONFIG              (*(volatile uint32_t*)(ND_RND_BASE + 0x00000504UL))         //Base address of RNG config
#define ND_RND_VALUE               (*(volatile uint32_t*)(ND_RND_BASE + 0x00000508UL))         //Base address of RNG value

#define ND_RND_EVNT_VALRDY         (*(volatile uint32_t*)(ND_RND_BASE + 0x00000100UL))         //Base address of RNG event ready

extern uint8_t nd_get_randomed_num(void);
extern void nd_rnd_init(void);

#endif
