#ifndef __NDEVICE_TWI
#define __NDEVICE_TWI

#include <stdbool.h>
#include <stdint.h>
#include <string.h>

typedef struct
{
  volatile uint32_t enable;
  volatile uint32_t pselscl;
  volatile uint32_t pselsda;
} stTWIconfig;

typedef struct
{
  volatile uint32_t intenset;
  volatile uint32_t intenclr;
} stTWIinterrupt;

typedef struct
{
  volatile uint32_t startRx;                //0x000
  volatile uint32_t dymmy0;
  volatile uint32_t startTx;                //0x008
  volatile uint32_t dymmy1[2];
  volatile uint32_t stop;                   //0x014
  volatile uint32_t dymmy2;
  volatile uint32_t suspend;                //0x01C
  volatile uint32_t resume;                 //0x020
} stTWIoperate;

typedef struct
{
  volatile uint32_t rxd;
  volatile uint32_t txd;
  volatile uint32_t dummy;
  volatile uint32_t freq;
} stTWIdata;

typedef struct
{
  uint8_t pBuff[40];
  uint8_t len;
  uint8_t index;
} stTWIbuffer;

#define ND_TWI0_BASE        (0x40003000UL)                      //Base address of TWI0
#define ND_TWI0_OPERATE     (ND_TWI0_BASE)                       //Base address of TWI start/stop
#define ND_TWI0_INTERRUPT   (ND_TWI0_BASE + 0x00000304UL)         //Base address of TWI interrupt
#define ND_TWI0_CONFIG      (ND_TWI0_BASE + 0x00000500UL)         //Base address of TWI config
#define ND_TWI0_DATA        (ND_TWI0_BASE + 0x00000518UL)         //Base address of TWI data
#define ND_TWI0_ADDRESS     (ND_TWI0_BASE + 0x00000588UL)           //Base address of TWI address register
  
#define ND_TWI0_EVNT_STOPPED        (ND_TWI0_BASE + 0x00000104UL)       //Base address of event stopped
#define ND_TWI0_EVNT_RXDREADY       (ND_TWI0_BASE + 0x00000108UL)       //Base address of event rxdready
#define ND_TWI0_EVNT_TXDSENT        (ND_TWI0_BASE + 0x0000011CUL)       //Base address of event txdsent
#define ND_TWI0_EVNT_ERROR          (ND_TWI0_BASE + 0x00000124UL)       //Base address of event error
#define ND_TWI0_EVNT_BB             (ND_TWI0_BASE + 0x00000138UL)       //Base address of event byte boundary
#define ND_TWI0_EVNT_SUSPENDEDS     (ND_TWI0_BASE + 0x00000148UL)       //Base address of event suspended

#define ND_TWI_INT_STOPPED         (0x00000001UL << 1)
#define ND_TWI_INT_RXDREADY        (0x00000001UL << 2)
#define ND_TWI_INT_TXDSENT         (0x00000001UL << 7)
#define ND_TWI_INT_ERROR           (0x00000001UL << 9)
#define ND_TWI_INT_BB              (0x00000001UL << 14)
#define ND_TWI_INT_SUSPENDED       (0x00000001UL << 18)

#define ND_TWI_ENABLE               (5UL)
#define ND_TWI_DISABLE              (0x00000000UL)

#define ND_TWI_FREQ_100K             (0x01980000UL)
#define ND_TWI_FREQ_250K             (0x04000000UL)
#define ND_TWI_FREQ_400K             (0x06680000UL)

extern bool nd_twi_init(uint32_t PinSCL, uint32_t PinSDA);

#endif
