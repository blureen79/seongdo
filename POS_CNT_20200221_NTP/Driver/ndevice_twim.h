/*
Author : Jaeyoung, LEE(NDevice.com)
Date : 03.18.2017
*/
#ifndef __NDEVICE_TWIM
#define __NDEVICE_TWIM

#include <stdbool.h>
#include <stdint.h>
#include <string.h>
#include <nrfx.h>

typedef struct
{
  volatile uint32_t enable;                 //0x500
  volatile uint32_t dummy;
  volatile uint32_t pselscl;
  volatile uint32_t pselsda;
} stTWIMconfig;

typedef struct
{
  volatile uint32_t intenable;
  volatile uint32_t intenset;
  volatile uint32_t intenclr;
} stTWIMinterrupt;

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
} stTWIMoperate;

typedef struct
{
  volatile uint8_t* rx_pointer;
  volatile uint32_t rx_maxcount;
  volatile uint32_t rx_amount;
  volatile uint32_t rx_list;
  volatile uint8_t* tx_pointer;
  volatile uint32_t tx_maxcount;
  volatile uint32_t tx_amount;
  volatile uint32_t tx_list;
} stTWIMdata;

#define ND_TWIM_BASE                (0x40004000UL)                      //Base address of TWIM0
#define ND_TWIM_OPERATE             (ND_TWIM_BASE)                       //Base address of TWI start/stop
#define ND_TWIM_INTERRUPT           (ND_TWIM_BASE + 0x00000300UL)         //Base address of TWI interrupt
#define ND_TWIM_CONFIG              (ND_TWIM_BASE + 0x00000500UL)         //Base address of TWI config
#define ND_TWIM_FREQUENCY           (ND_TWIM_BASE + 0x00000524UL)           //Base address of TWI frequency register
#define ND_TWIM_DATA                (ND_TWIM_BASE + 0x00000534UL)         //Base address of TWI data registers
#define ND_TWIM_ADDRESS             (ND_TWIM_BASE + 0x00000588UL)           //Base address of TWI address register

#define ND_TWIM_SHORTS              (*(volatile uint32_t*)(ND_TWIM_BASE + 0x00000200UL))         //Base address of TWI shortcut
#define ND_TWIM_EVNT_STOPPED        (*(volatile uint32_t*)(ND_TWIM_BASE + 0x00000104UL))       //Base address of event stopped
#define ND_TWIM_EVNT_ERROR          (*(volatile uint32_t*)(ND_TWIM_BASE + 0x00000124UL))       //Base address of event error
#define ND_TWIM_EVNT_SUSPENDED      (*(volatile uint32_t*)(ND_TWIM_BASE + 0x00000148UL))       //Base address of event suspended
#define ND_TWIM_EVNT_RXDREADY       (*(volatile uint32_t*)(ND_TWIM_BASE + 0x0000014CUL))       //Base address of event rxdready
#define ND_TWIM_EVNT_TXDSENT        (*(volatile uint32_t*)(ND_TWIM_BASE + 0x00000150UL))       //Base address of event txdsent
#define ND_TWIM_EVNT_LASTRX         (*(volatile uint32_t*)(ND_TWIM_BASE + 0x0000015CUL))       //Base address of event lastrx
#define ND_TWIM_EVNT_LASTTX         (*(volatile uint32_t*)(ND_TWIM_BASE + 0x00000160UL))       //Base address of event lasttx

#define ND_TWIM_INT_STOPPED         (0x00000001UL << 1)
#define ND_TWIM_INT_ERROR           (0x00000001UL << 9)
#define ND_TWIM_INT_SUSPENDED       (0x00000001UL << 18)
#define ND_TWIM_INT_RXSTARTED       (0x00000001UL << 19)
#define ND_TWIM_INT_TXSTARTED       (0x00000001UL << 20)
#define ND_TWIM_INT_LASTRX          (0x00000001UL << 23)                //Reached to last pointer
#define ND_TWIM_INT_LASTTX          (0x00000001UL << 24)                //Reached to last pointer

#define ND_TWIM_SHORT_LTSR          (0x00000001UL << 7)                 //LASTTX and STARTRX
#define ND_TWIM_SHORT_LTSU          (0x00000001UL << 8)                 //LASTTX and SUSPENDED
#define ND_TWIM_SHORT_LTSP          (0x00000001UL << 9)                 //LASTTX and STOP
#define ND_TWIM_SHORT_LRST          (0x00000001UL << 10)                //LASTRX and STARTTX
#define ND_TWIM_SHORT_LRSP          (0x00000001UL << 12)                //LASTRX and STOP

#define ND_TWIM_ENABLE               (6UL)                        //For TWIM
//#define ND_TWIM_ENABLE               (5UL)                          //For TWI
#define ND_TWIM_DISABLE              (0x00000000UL)

#define ND_TWIM_FREQ_100K             (0x01980000UL)
#define ND_TWIM_FREQ_250K             (0x04000000UL)
#define ND_TWIM_FREQ_400K             (0x06400000UL)

typedef void (*fnReceiveHandler)(void);

extern bool nd_twim_init(uint32_t PinSCL, uint32_t PinSDA, stTWIMdata cfg_data);
extern bool twim_repeateddata(uint8_t addr, volatile uint8_t* pTxBuf, uint8_t TxLen, volatile uint8_t* pRxBuf, uint8_t RxLen, fnReceiveHandler handler);
extern void twim_rcvdata(uint8_t addr, volatile uint8_t* pBuf, uint8_t len, fnReceiveHandler handler);
extern bool twim_senddata(uint8_t addr, volatile uint8_t* pBuf, uint8_t len);
extern void twim_clear_busy(void);
extern void nd_twim_gen_stop(void);
extern bool twim_IsBusy(void);

#endif
