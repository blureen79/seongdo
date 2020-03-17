#ifndef __PROCESS_DATA
#define __PROCESS_DATA

#include <stdint.h>
#include "ble_nus.h"
#include "func.h"

extern void RFRCV_Handler(uint8_t cmd, uint8_t* pData);
extern void cb_app_timer_handler(void* params);
extern void parser_data(uint8_t const* pBuff, uint16_t len, ble_nus_t* m_nus, uint16_t handle);
extern void unqueue_wireless(void);
extern void gen_update_packet(void);
extern void gen_update_fail_packet(const char *pStr);
extern void setUpdateStat(uint8_t val);
extern void init_process(void);
extern bool send_queuedQRData(void);
extern void send_endQR(void);
extern int setBase64Buffer(uint8_t *inBuff, uint16_t len);
extern void setQRCodeData(uint16_t index);
extern void sendCAT2Succeed(void);
extern void clearEncBusy(void);
extern void clear_receipt(void);
extern void set_nus_ptr(ble_nus_t* pNus);
extern void clearEncBusy(void);

extern bool IsAdvertised(void);             //from main.c

extern stBLEQueue bleQueue_g;

extern bool send_receiptData(void);
extern void send_endPRS(void);
extern void send_modeSum(bool parsing);
extern void send_responseToBLE(char *pStr);
extern void send_responseToBLEinEvent(char *pStr);
extern void send_PRS(void);
extern void send_DIS(void);
extern void send_PRF(void);
extern void send_FUM(void);
extern void print_system_prameter(bool change);

extern void init_parameter(void);
extern uint16_t process_is_there_packet(void);
extern void process_wait_complete_packet(void);
#endif
