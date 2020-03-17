/*
Author : Jaeyoung, LEE(NDevice.com)
Date : 06.21.2017
*/
#ifndef __NDEVICE_RF
#define __NDEVICE_RF

#include <stdbool.h>
#include <stdint.h>
#include <string.h>
#include <nrf_sdm.h>
#include "nrf.h"
#include "nrf_drv_common.h"
#include "ndevice_util.h"
#include "nrf_gpio.h"
#include "nrf_delay.h"
#include "ndevice_rnd.h"
#include "process_data.h"

#define ND_RF_BASE                (0x40001000UL)                        //Base address of RF
#define ND_RF_TASK                (ND_RF_BASE)                          //Base address of RF tasks
#define ND_RF_INTERRUPT           (ND_RF_BASE + 0x00000304UL)           //Base address of RF interrupt
#define ND_RF_INFO                (ND_RF_BASE + 0x00000400UL)           //Base address of RF informations, CRC, match
#define ND_RF_PACK_CFG            (ND_RF_BASE + 0x00000504UL)           //Base address of RF packet configurations
#define ND_RF_DEVICE              (ND_RF_BASE + 0x00000600UL)           //Base address of RF device address
#define ND_RF_MODECNF0            (*(volatile uint32_t *)(ND_RF_BASE + 0x00000650UL))          //Base address of RF mode configuration

#define ND_RF_POWER               (*(volatile uint32_t *)(ND_RF_BASE + 0x00000FFCUL))          //Base address of RF peripheral power control

#define ND_RF_EVNT_READY          (*(volatile uint32_t*)(ND_RF_BASE + 0x00000100UL))       //Base address of event ready to be started
#define ND_RF_EVNT_ADDR           (*(volatile uint32_t*)(ND_RF_BASE + 0x00000104UL))       //Base address of event address sent or recieved
#define ND_RF_EVNT_PAYLOAD        (*(volatile uint32_t*)(ND_RF_BASE + 0x00000108UL))       //Base address of event payload sent or recieved
#define ND_RF_EVNT_END            (*(volatile uint32_t*)(ND_RF_BASE + 0x0000010CUL))       //Base address of event packet sent or received
#define ND_RF_EVNT_DISABLED       (*(volatile uint32_t*)(ND_RF_BASE + 0x00000110UL))       //Base address of event address sent or recieved
#define ND_RF_EVNT_DEVMATCH       (*(volatile uint32_t*)(ND_RF_BASE + 0x00000114UL))       //Base address of event device address matched
#define ND_RF_EVNT_DEVMISS        (*(volatile uint32_t*)(ND_RF_BASE + 0x00000118UL))       //Base address of event no device address matched
#define ND_RF_EVNT_RSSIEND        (*(volatile uint32_t*)(ND_RF_BASE + 0x0000011CUL))       //Base address of event sampling rssi completed
#define ND_RF_EVNT_BCMATCH        (*(volatile uint32_t*)(ND_RF_BASE + 0x00000128UL))       //Base address of event bit counter reaced bit count value
#define ND_RF_EVNT_CRCOK          (*(volatile uint32_t*)(ND_RF_BASE + 0x00000130UL))       //Base address of event CRC is ok
#define ND_RF_EVNT_CRCERROR       (*(volatile uint32_t*)(ND_RF_BASE + 0x00000134UL))       //Base address of event CRC is not ok

#define ND_RF_SHORTS              (*(volatile uint32_t*)(ND_RF_BASE + 0x00000200UL))       //Base address of RF shortcuts

#define ND_RF_STATE_DISABLED                    0x00
#define ND_RF_STATE_RXRU                        0x01
#define ND_RF_STATE_RXIDLE                      0x02
#define ND_RF_STATE_RX                          0x03
#define ND_RF_STATE_RXDIS                       0x04            //Rx disabled
#define ND_RF_STATE_TXRU                        0x09
#define ND_RF_STATE_TXIDLE                      0x0A
#define ND_RF_STATE_TX                          0x0B
#define ND_RF_STATE_TXDIS                       0x0C            //Tx disabled

#define ND_RF_INT_READY                         (0x00000001UL)
#define ND_RF_INT_ADDRESS                       (0x00000001UL << 1)
#define ND_RF_INT_PAYLOAD                       (0x00000001UL << 2)
#define ND_RF_INT_END                           (0x00000001UL << 3)
#define ND_RF_INT_DISABLED                      (0x00000001UL << 4)
#define ND_RF_INT_DEVMATCH                      (0x00000001UL << 5)

#define ND_RF_SHORTS_RDST                       (0x00000001UL)                   //Ready start
#define ND_RF_SHORTS_EDDI                       (0x00000001UL << 1)              //END disable
#define ND_RF_SHORTS_DITX                       (0x00000001UL << 2)              //Disabled TX
#define ND_RF_SHORTS_DIRX                       (0x00000001UL << 3)              //Disabled RX
#define ND_RF_SHORTS_ADRS                       (0x00000001UL << 4)              //Address RSSI starat
#define ND_RF_SHORTS_EDST                       (0x00000001UL << 5)              //END start
#define ND_RF_SHORTS_ADBC                       (0x00000001UL << 6)              //ADDRESS BCSTART
#define ND_RF_SHORTS_DIRS                       (0x00000001UL << 8)              //DISABLED RSSI stop

#define ND_RF_TXPWR_p4dBm                       0x04
#define ND_RF_TXPWR_p3dBm                       0x03
#define ND_RF_TXPWR_0dBm                        0x00
#define ND_RF_TXPWR_n4dBm                       0xFC
#define ND_RF_TXPWR_n8dBm                       0xF8
#define ND_RF_TXPWR_n12dBm                      0xF4
#define ND_RF_TXPWR_n16dBm                      0xF0
#define ND_RF_TXPWR_n20dBm                      0xEC
#define ND_RF_TXPWR_n30dBm                      0xD8                //-40dBm
#define ND_RF_TXPWR_n40dBm                      0xD8                //-40dBm

#define ND_RF_MODE_NRF1M                        (0UL)
#define ND_RF_MODE_NRF2M                        (1UL)
#define ND_RF_MODE_NRF250K                      (2UL)
#define ND_RF_MODE_BLE1M                        (3UL)
#define ND_RF_MODE_BLE2M                        (4UL)

#define ND_RF_MODECNF0_FASTMODE                 (1UL)
#define ND_RF_MODECNF0_CENTER                   (0x00000002UL << 8)

#define ND_RF_SW_TX                           0x00
#define ND_RF_SW_RX                           0x01

#define ADV_CHANNEL_ADDR               			(0x8E89BED6UL)

#define BLE_AD_TYPE_SHORTNAME                   0x08

#define ND_RF_RXBUFFER_SIZE                     30

typedef struct
{
  volatile uint32_t txen;                   //0x000
  volatile uint32_t rxen;                   //0x004
  volatile uint32_t start;                  //0x008
  volatile uint32_t stop;                   //0x00c
  volatile uint32_t disable;                //0x010
  volatile uint32_t rssistart;              //0x014
  volatile uint32_t rssistop;               //0x018
  volatile uint32_t bcstart;                //0x01c
  volatile uint32_t bcstop;                 //0x020
} stRFtask;

typedef struct
{
  volatile uint32_t intenset;                //0x304
  volatile uint32_t intenclr;                //0x308
} stRFinterrupt;

typedef struct
{
  volatile uint32_t crcstatus;              //0x400
  volatile uint32_t dummy0;                 //0x404
  volatile uint32_t rxmatch;                //0x408
  volatile uint32_t rxcrc;                  //0x40C
  volatile uint32_t dai;                    //0x410
} stRFinfo;

typedef struct
{
  volatile uint32_t packetptr;              //0x504
  volatile uint32_t frequency;              //0x508
  volatile uint32_t txpower;                //0x50c
  volatile uint32_t mode;                   //0x510
  volatile uint32_t pcnf0;                  //0x514
  volatile uint32_t pcnf1;                  //0x518
  volatile uint32_t base0;                  //0x51c
  volatile uint32_t base1;                  //0x520
  volatile uint32_t prefix0;                //0x524
  volatile uint32_t prefix1;                //0x528
  volatile uint32_t txaddress;              //0x52c
  volatile uint32_t rxaddress;              //0x530
  volatile uint32_t crccnf;                 //0x534
  volatile uint32_t crcpoly;                //0x538
  volatile uint32_t crcinit;                //0x53c
  volatile uint32_t dummy0;                 //0x540
  volatile uint32_t tifs;                   //0x544
  volatile uint32_t rssisample;             //0x548
  volatile uint32_t dummy1;                 //0x54c
  volatile uint32_t state;                  //0x550
  volatile uint32_t datawhieteiv;           //0x554
  volatile uint32_t dummy2[2];              //0x558~0x55c
  volatile uint32_t bcc;                    //0x560
} stRFpacket;

typedef struct
{
  volatile uint32_t dab[8];              //0x600~0x61c
  volatile uint32_t dap[8];              //0x620~0x63c
  volatile uint32_t dacnf;               //0x640
} stRFdevice;

typedef struct
{
  uint8_t chType;                 //Type of channel selector, Auto channel or Manual channel
  uint8_t len;                  //length of this packet
  uint8_t type;                 //packet tyep, it must be added to len
  uint8_t* pData;               //pointer of data
  uint8_t status;               //resend, sent, ready
} stBLEdata;

typedef struct
{
  uint8_t data[ND_RF_RXBUFFER_SIZE][32];
  uint8_t idx_header;
  uint8_t idx_tail;
} stRXBuffer;

typedef enum
{
  RF_RU = 0x00,
  RF_READY,
  RF_TX,
  RF_RX,
  RF_DISABLED,
  RF_IDLE,
  RF_RESEND
} enumRFflowctrl;

typedef struct
{
  uint8_t Ver;
  uint8_t Com_name;
  uint16_t ComID;
  uint8_t BroadID;
  uint8_t Cmd;
  uint8_t Data[10];
} stPacketParser;

typedef struct
{
  uint8_t old_status;
  uint8_t status;
} stRFFSM;

typedef enum
{
  AUTO_ADV = 0x00,
  MANU_ADV
} enumRF;

typedef enum
{
  AUTOSEND_SEND,
  AUTOSEND_WAIT,
  AUTOSEND_RX,
  AUTOSEND_READY,
  AUTOSEND_FINISH,
  AUTOSEND_RCV
} enumAutoSend;
    
typedef void    (*fnRF_RCVHandler)(uint8_t cmd, uint8_t* pData);

extern void nd_rf_init(fnRF_RCVHandler mainRCVHandler);
extern void nd_rf_set_packet(void);
extern void nd_rf_set_packet(void);
extern uint8_t nd_rf_autoadvsend(uint8_t *pData, uint8_t len);
extern uint8_t nd_rf_advsend(uint8_t *pData, uint8_t len, uint8_t channel);
extern void nd_rf_rcv(uint32_t timeout, uint8_t *pRcv, uint8_t *pLen);
extern uint8_t nd_rf_get_rfstatus(void);
extern uint8_t nd_rf_IsBusy(void);
extern void nd_rf_channel(uint8_t ch);
extern uint8_t nd_rf_resend(void);
extern void nd_rf_autosend_fsm(uint8_t reset);
extern char nd_rf_nextchannel(bool reset);
extern void nd_rf_ch_handler(void);
extern uint8_t nd_rf_resend_with_ch(uint8_t *pData, uint8_t channel);
extern void dbg_nd_rf_rcv(void);
extern void nd_rf_disable_RF(void);
extern void nd_rf_rawsend(uint8_t *pData, uint8_t len);
#endif
