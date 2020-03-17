/*
Author : Jaeyoung, LEE(NDevice.com)
Date : Jan.12.2019
Custom board configurations
*/
#ifndef __NDEVICE_BOARD
#define __NDEVICE_BOARD

#define DEF_SPI_SRAM_CS                 (20U)
#define DEF_SPI_SRAM_SCK                (18U)
#define DEF_SPI_SRAM_MISO               (19U)
#define DEF_SPI_SRAM_MOSI               (17U)
#define DEF_SPI_SRAM_HOLD               (16U)

#define DEF_SOUND_SDO                    (2U)
#define DEF_SOUND_SCK                    (3U)
#define DEF_AUDIO_RESET                  (8U)
#define DEF_AUDIO_BUSY                   (7U)

#define DEF_GPIO_RED1_PWM                (30U)
#define DEF_GPIO_SMDLED_D1                (4U)
#define DEF_GPIO_AUDIOSHDN               (31U)

#define DEF_GPIO_BUTTON                 (22U)

#define CAT2_PIN_NUMBER                 (12U)           //
#define RX_PIN_NUMBER                   (11U)           //

#define PRN_RX_PIN_NUMBER               (14U)           //
#define TX_PIN_NUMBER                   (13U)           //

//ADG1436 Switch IC Driection Select
#define RS232_IN1						(23U)
#define RS232_IN2			            (24U)

//I2C
#define I2C_SDA				            (5U)
#define I2C_SCL				            (6U)

#define POS_DIR							(25U)

//HC595 Write Define
#define CL1									(9U)
#define CL2									(10U)
#define SHIFT_DATA					(26U)
#define SHIFT_CLOCK					(27U)
#define LATCH_CLOCK					(31U)

#endif
