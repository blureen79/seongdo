#ifndef __NDEVICE_SPI
#define __NDEVICE_SPI

#include <stdbool.h>
#include <stdint.h>
#include <string.h>

typedef struct
{
  uint8_t pBuff[40];
  uint8_t len;
  uint8_t index;
} stSPIbuffer;

typedef struct
{
  volatile uint8_t* rxd_ptr;
  volatile uint32_t rxd_maxcnt;
  volatile uint32_t rxd_amount;
  volatile uint32_t rxd_list;
  volatile uint8_t* txd_ptr;
  volatile uint32_t txd_maxcnt;
  volatile uint32_t txd_amount;
  volatile uint32_t txd_list;
} stSPIData;

typedef struct
{
  volatile uint32_t sck;
  volatile uint32_t mosi;
  volatile uint32_t miso;
} stSPIpsel;

typedef struct
{
  volatile uint32_t set;
  volatile uint32_t clr;
} stSPIinterrupt;

typedef void (*fnSPIReceiveHandler)(void);

#define ND_SPI1_BASE        (0x40004000UL)                      //Base address of SPI1
#define ND_SPI1_OPERATE     (ND_SPI1_BASE + 0x00000010UL)                       //Base address of SPI start/stop
#define ND_SPI1_PSEL        (ND_SPI1_BASE + 0x00000508UL)         //Base address of SPI psel
#define ND_SPI1_INTERRUPT   (ND_SPI1_BASE + 0x00000304UL)       //Base address of SPI interrupt
#define ND_SPI1_DATA        (ND_SPI1_BASE + 0x00000534UL)       //Base address of SPI interrupt
  
#define ND_SPI1_SHORTS              (*(volatile uint32_t*)(ND_SPI1_BASE + 0x00000200UL))       //Base address of SPI shorts
#define ND_SPI1_EVNT_STOPPED        (*(volatile uint32_t*)(ND_SPI1_BASE + 0x00000104UL))       //Base address of event stopped
#define ND_SPI1_EVNT_ENDRX          (*(volatile uint32_t*)(ND_SPI1_BASE + 0x00000110UL))       //Base address of event END RX
#define ND_SPI1_EVNT_END            (*(volatile uint32_t*)(ND_SPI1_BASE + 0x00000118UL))       //Base address of event END
#define ND_SPI1_EVNT_ENDTX          (*(volatile uint32_t*)(ND_SPI1_BASE + 0x00000120UL))       //Base address of event END TX
#define ND_SPI1_EVNT_STARTED        (*(volatile uint32_t*)(ND_SPI1_BASE + 0x0000014CUL))       //Base address of event started

#define ND_SPI1_ENABLE              (*(volatile uint32_t*)(ND_SPI1_BASE + 0x00000500UL))       //Base address of SPI Enable
#define ND_SPI1_FREQ                (*(volatile uint32_t*)(ND_SPI1_BASE + 0x00000524UL))       //Base address of SPI frequency
#define ND_SPI1_CONFIG              (*(volatile uint32_t*)(ND_SPI1_BASE + 0x00000554UL))       //Base address of SPI config

#define ND_SPI1_START               (*(volatile uint32_t*)(ND_SPI1_BASE + 0x00000010UL))       //Start data process
#define ND_SPI1_TASKSTOP            (*(volatile uint32_t*)(ND_SPI1_BASE + 0x00000014UL))       //Stop the TASK

#define ND_SPI1_SHORT_ENDSTART      (0x00000001UL << 17)                                       //Enable shortcut for END_START
#define ND_SPI1_EN_ENABLE              (0x00000007UL)                                             //SPIM enable

#define ND_SPI_INT_END              (0x00000001UL << 6)

#define ND_SPI_FREQ_K125             (0x02000000UL)
#define ND_SPI_FREQ_K250             (0x04000000UL)
#define ND_SPI_FREQ_K500             (0x08000000UL)
#define ND_SPI_FREQ_M1               (0x10000000UL)
#define ND_SPI_FREQ_M2               (0x20000000UL)
#define ND_SPI_FREQ_M4               (0x40000000UL)
#define ND_SPI_FREQ_M8               (0x80000000UL)

extern bool nd_spi_init(uint32_t PinSCK, uint32_t PinSDI, uint32_t PinSDO, uint32_t PinSCS);
extern void spim_repeateddata(uint8_t* pTxBuf, uint32_t TxLen, uint8_t* pRxBuf, uint32_t RxLen, fnSPIReceiveHandler handler);
extern void spim_senddata(uint8_t* pBuf, uint32_t len);
extern uint8_t IsSPIBusy(void);
#endif
