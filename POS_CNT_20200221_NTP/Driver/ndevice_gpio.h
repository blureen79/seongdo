#ifndef __NDEVICE_GPIO
#define __NDEVICE_GPIO

#include <stdbool.h>
#include <stdint.h>
#include <string.h>
#include <nrfx.h>

#define DEF_GPIO_CONFIG_REN                 (0UL)               //Read enabled
#define DEF_GPIO_CONFIG_WEN                 (1UL)               //Write enabled

#define ND_GPIO_BASE                (0x4001E000UL)                          //Base address of NVMC
#define ND_GPIO_ERALL               (ND_NVMC_BASE + 0x0000050CUL)           //Base address of Erase all

#define ND_GPIO_READY               (*(volatile uint32_t*)(ND_NVMC_BASE + 0x00000400UL))       //Base address of ready

extern void nd_gpio_init(void);
#endif
