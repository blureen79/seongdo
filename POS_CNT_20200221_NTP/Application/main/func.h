#ifndef __NDEVICE_FUNC
#define __NDEVICE_FUNC

#include <stdint.h>
#include <string.h>
#include <stdbool.h>
#include "ndevice_eeprom.h"
#include "shoponair.h"

//#define DEF_BOOT_START                  (0x00039000U)       //Address of bootloader data           Page size is 4096(0x1000) bytes
//#define DEF_BOOT_FLAG_ADDR              (0x00059000U)       //Address of bootloader data           Page size is 4096(0x1000) bytes
//#define DEF_BOOT_FLAGSIZE               (16U)
//#define DEF_BOOT_LIMITSIZE              (86016U)               //84 kbytes, 84 x 1024 bytes

//#define BLE_QUEUE_SIZE                         64
#define BLE_QUEUE_SIZE                         140

typedef enum
{
  SYSEVT_SEND_BLEDATA   = 0x0001,
  SYSEVT_SEND_RECEIPT   = 0x0002,
  SYSEVT_WRITE_QRDATA = 0x0004,
  SYSEVT_WRITE_REVDATA = 0x0008,
  SYSEVT_WRITE_OPT      = 0x0010,
  SYSEVT_DIS_CONNECT    = 0x0020,
  SYSEVT_BLE_DISCONNECT = 0x0040,
  SYSEVT_TIMEOUTSTART   = 0x0080,

  SYSEVT_ADV_START      = 0x0100,
  SYSEVT_ADV_STOP       = 0x0200,
  SYSEVT_CLEAR_RECEIPT  = 0x0400,
  SYSEVT_BLINK_LED      = 0x0800,
  SYSEVT_BTN_PUSH       = 0x1000,
  SYSEVT_PLAY_SOUND     = 0x2000,
  SYSEVT_PRINT_RECEIPT  = 0x4000,
  SYSEVT_FORCE_PRINT    = 0x8000,
  SYSEVT_ALL = 0xFFFF
} enumSystemEvent;

typedef enum
{
  CFG_BAUD_9600 = 0x00,
  CFG_BAUD_19200 = 0x01,
  CFG_BAUD_38400 = 0x02,
  CFG_BAUD_57600 = 0x03,
  CFG_BAUD_115200 = 0x04,
  CFG_MODE_POS = 0x05,
  CFG_MODE_CAT = 0x06,
  CFG_MODE_KEYPAD = 0x07,
  CFG_PRINT_A = 0x08,
  CFG_PRINT_B = 0x09,
  CFG_PRINT_C = 0x0A
} enumCFG;

typedef enum
{
  QUEUE_QRCODE = 0x00,
  QUEUE_QRCODE_END,
  QUEUE_RECEIPT,
  QUEUE_RECEIPT_END,
  QUEUE_STANDBY
} enumBLEQueue;

typedef enum
{
  RECEIPT_FSM_READY = 0x0001,
  RECEIPT_FSM_READ = 0x0002,
  RECEIPT_FSM_WRITE = 0x0004,
  RECEIPT_FSM_AVAILABLE = 0x0008,
  RECEIPT_FSM_CLEAN = 0x0010
} enumReceiptState;

typedef struct
{
  uint8_t mode;
  uint8_t baudrate;
  uint8_t keypad_baudrate;
  uint8_t volume;
  uint8_t print_type;
  uint8_t auto_cut;
	
  bool IsReceiptdata;
  bool IsTagReceived;                   //when received 'tag' command
  bool IsReceiptCompleted;
  bool buttonEvent;

  bool sendDIS;
  bool sendPRS;
  bool sendPRF;
  bool ble_tag;
  bool ble_wait;
  bool ble_prf;
  bool ble_opt;
  bool ble_cancel;
  bool ble_print;
  bool ble_qr;
  bool ble_qrs;
  bool ble_cpk;
  bool ble_tpk;
  bool ble_rd;
  bool ble_wr_rd;
  bool ble_qd;
  bool ble_prs0;
  bool ble_prs1;
  bool qr_send_complete;
  bool print;
	
  bool ble_tag_connection;	//0209 KSD.MODIFY
  bool rssi_limit;	//RSSI LIMIT 초과 조건을 감지한는 조건
#ifdef ENABLE_SOA_DFU  
  bool ble_fum;
  bool soa_dfu;
#endif  
  bool seg_display;
} stPOSCVTcfg;

typedef struct
{
  uint16_t idx_hdr;
  uint16_t idx_tail;
  uint8_t data[EEP_QRDATA_LIMIT + 1];
} stQRData;

typedef struct
{
  bool updatAvailable; 
  uint32_t nextAddress;
  uint8_t stat;
  uint16_t last_sent;
  uint32_t checkSum;
} stUpdateProcess;

typedef struct
{
  uint16_t data_idx_hdr;
  uint16_t data_idx_tail;
  stQRData QRData;
  uint16_t base64Len;
  uint8_t base64Buff[BLE_QUEUE_SIZE + 1];
  uint8_t tempCAT2Buff[20 + 1];
  uint8_t CAT2Buff[20 + 1];
  uint8_t CAT2Len;
  bool IsBusy;
  uint8_t state;
  bool sentSuccess;
} stBLEQueue;

extern stPOSCVTcfg PoscvtCfg_g;

extern void stick_fsm(bool reset);
extern void cb_set_IMU_sending(bool set, uint8_t param);
extern void cb_tick_handler(void);
extern void cb_all_stop(void);
extern void set_SysEvent(uint16_t event);
extern void clr_SysEvent(uint16_t event);
extern void set_MotorParam(uint8_t count, uint8_t on_time, uint8_t off_time);
extern void set_MotorPower(uint8_t pwr);
extern void setBLUELEDrate(uint8_t bright);
extern void setREDLEDrate(uint8_t bright);
extern void set_stickID(uint8_t id);
extern uint8_t get_stickID(void);
extern void pwm_init(void);
extern void setBLUELEDparam(uint8_t count, uint16_t on_time, uint16_t off_time);
extern void setREDLEDparam(uint8_t count, uint16_t on_time, uint16_t off_time);
extern void send_battery_info(void);
extern void button_check(void);
extern void set_AppConnected(bool value);
extern void setLASERparam(uint8_t rate);
extern void init_stick_var(void);
extern uint32_t get_tickcount(void);
extern uint16_t get_battery_soc(bool chager_en);
extern void init_fstorage(void);
extern bool nd_erase_flash_opt(uint32_t NoOfPages);
extern bool nd_erase_flash(uint32_t NoOfPages);
extern bool nd_write_flash(uint32_t * p_dst, uint32_t const * p_src, uint32_t size);
extern void set_stick_sleep(void);
extern void clear_sleep_time(void);
extern void set_stick_wakeup(void);
extern bool nd_IsErased(void);
extern void ctrlLED(uint8_t sel, uint16_t bright);
extern void setLastFalshInfo(uint32_t address, uint32_t* pBuff);
extern void bytearrToUINT32(uint8_t *pBuff, uint32_t *pOut);
extern void sleep_sensors(void);
extern void setMagCalForce(void);
extern void gpiote_init(void);
extern void checkChargeState(void);
extern bool isConnected(void);
extern void stop_all_pwm(void);
extern void start_all_pwm(void);
extern void print_receipt(void);
extern unsigned char new_print_receipt(bool first);
extern void setQRData(uint8_t *pInBuff, uint16_t len);
extern void setReverseData(uint8_t *pInBuff, uint16_t len, bool reset);
extern void setSoundTotal(uint32_t value);
extern void getConfig(stPOSCVTcfg* cfg);
extern void soundBeep(uint8_t loopCnt);
extern void setReceiptData(void);
extern void writeReverseToSRAM(void);
extern void resetQRWriteTimer(void);
extern void resetRemainConnect(void);
extern void checkRemainConnect(void);
extern void play_sound_mode(uint32_t value);
extern void clrTagReceived(void);
extern void clrTimeout(void);
extern void setTimeoutStart(void);
extern void setCAT2Data(uint8_t *pInBuff, uint16_t len);
extern bool receiptFSM(uint16_t state);
extern uint16_t getReceiptFSM(void);
extern void setVolume(uint32_t value);
extern void set_print_baud_rate(uint32_t value);
extern void set_print_mode(uint8_t value);

extern uint16_t ble_get_handle_state(void);         //in the main.c
extern void timers_stop(void);                      //in the main.c
extern bool IsConnected(void);                      //in the main.c
extern void set_advertising_active(void);
extern void clear_advertising_active(void);
extern void resetOPTWriteTimer(void);
extern void clear_print_tick(void);
#endif

