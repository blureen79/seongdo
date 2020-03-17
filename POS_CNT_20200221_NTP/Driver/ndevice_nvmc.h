#ifndef __NDEVICE_NVMC
#define __NDEVICE_NVMC

#include <stdbool.h>
#include <stdint.h>
#include <string.h>
#include "nrf.h"

#define DEF_USERUICR_BASEADDR                   (0x10001080UL)

#define DEF_NVMC_CONFIG_REN                 (0UL)               //Read enabled
#define DEF_NVMC_CONFIG_WEN                 (1UL)               //Write enabled
#define DEF_NVMC_CONFIG_EEN                 (2UL)               //Erase enabled

#define ND_NVMC_BASE                (0x4001E000UL)                          //Base address of NVMC
#define ND_NVMC_ERALL               (ND_NVMC_BASE + 0x0000050CUL)           //Base address of Erase all

#define ND_NVMC_READY               (*(volatile uint32_t*)(ND_NVMC_BASE + 0x00000400UL))       //Base address of ready
#define ND_NVMC_CONFIG              (*(volatile uint32_t*)(ND_NVMC_BASE + 0x00000504UL))       //Base address of config
#define ND_NVMC_ERPAGE              (*(volatile uint32_t*)(ND_NVMC_BASE + 0x00000508UL))       //Base address of Erase page
#define ND_NVMC_ERUICR              (*(volatile uint32_t*)(ND_NVMC_BASE + 0x00000514UL))           //Base address of Erase UICR

extern bool is_busy_nvmc(void);
extern void nd_nvmc_cfg(uint32_t cfg);
extern void nd_nvmc_write(uint32_t addr, uint32_t* pData, uint8_t len);
extern uint32_t nd_nvmc_read(uint32_t addr);
extern void nd_nvmc_erase(uint32_t addr);
extern uint32_t getUICR(uint32_t addr);
extern void setUICR(uint32_t addr, uint32_t data);
extern void nd_uicr_erase(void);
#endif
