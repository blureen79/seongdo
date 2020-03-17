/*
Author : Jaeyoung, LEE(NDevice.com)
Date : Sep.02.2017
AT25SF041 is using MSB, 24bit Address
*/
#ifndef __EEPROM
#define __EEPROM

#include <stdbool.h>
#include <stdint.h>
#include <string.h>
#include "ndevice_spi.h"

#define WRITE_MODE_BYTE                     0x00
#define WRITE_MODE_PAGE                     0x80
#define WRITE_MODE_SEQ                      0x40

#define EEP_BUFF_SIZE                       512

#define FLASH_OPTION_ADDR               (0x00060000U)
#define FLASH_QRCODE_ADDR               (0x00062000U)
#define FLASH_QRCODE_LIMIT              (4096U)             //4kBytes

//List of command
#define EEP_CMD_READARRAY                                 0x03
#define EEP_CMD_ERASEBLK_4K                               0x20
#define EEP_CMD_ERASEBLK_32K                              0x52
#define EEP_CMD_ERASEBLK_64K                              0xD8
#define EEP_CMD_ERASECHIP                                 0x60
#define EEP_CMD_WRITE_EN                                  0x06
#define EEP_CMD_WRITE_DIS                                 0x04
#define EEP_CMD_WRITE                                     0x02
#define EEP_CMD_DEVICEID                                  0x9F
#define EEP_CMD_STATUS1                                   0x05
#define EEP_CMD_STATUS2                                   0x35
#define EEP_CMD_WRSR                                      0x01
//End of command

extern void eep_read_array(uint16_t addr, uint8_t *pOutBuff, uint32_t len, fnSPIReceiveHandler handler);
extern void eep_read_byte(uint16_t addr, fnSPIReceiveHandler handler);
extern void eep_write_array(uint16_t addr, uint8_t* pInBuff, uint32_t len);
extern void eep_write_enable(uint8_t setEnable);
extern void eep_erase(uint8_t block_size);
extern void eep_write_status(uint8_t status);
#endif
