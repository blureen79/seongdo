#ifndef __NDEVICE_SADC
#define __NDEVICE_SADC

#include <stdbool.h>
#include <stdint.h>
#include <string.h>

typedef struct
{
  volatile uint32_t pselp;
  volatile uint32_t pseln;
  volatile uint32_t config;
  volatile uint32_t limit;
} stSADCchannel;

typedef struct
{
  volatile uint32_t resolution;
  volatile uint32_t oversample;
  volatile uint32_t samplerate;
} stSADCconfig;

typedef struct
{
  volatile uint32_t inten;
  volatile uint32_t intenset;
  volatile uint32_t intenclr;
} stSADCinterrupt;

typedef struct
{
  volatile uint32_t start;
  volatile uint32_t sample;
  volatile uint32_t stop;
  volatile uint32_t cal_offset;
} stSADCoperate;

typedef struct
{
  volatile uint32_t pResult;
  volatile uint32_t maxcnt;
  volatile uint32_t amount;
} stSADCresult;

typedef struct
{
  uint8_t num_of_ch;
  bool  done;
} stADCcontrol;

#define ND_SADC_BASE         (0x40007000UL)                      //Base address of ADC register
#define ND_SADC_OPERATE      (ND_SADC_BASE)                       //Base address of ADC start/stop
#define ND_SADC_INTERRUPT    (ND_SADC_BASE + 0x00000300UL)         //Base address of ADC interrupt
#define ND_SADC_SET_CH       (ND_SADC_BASE + 0x00000510UL)         //Base address of ADC config
#define ND_SADC_CONFIG       (ND_SADC_BASE + 0x000005F0UL)         //Base address of ADC config
#define ND_SADC_RESULT       (ND_SADC_BASE + 0x0000062CUL)         //Result of ADC

#define ND_SADC_ENABLE       (*(volatile uint32_t*)(ND_SADC_BASE + 0x00000500UL))       //SADC enable

#define ND_SADC_EVNT_END            (*(volatile uint32_t*)(ND_SADC_BASE + 0x00000104UL))       //Base address of event conversion end
#define ND_SADC_EVNT_DONE           (*(volatile uint32_t*)(ND_SADC_BASE + 0x00000108UL))       //Base address of event conversion done
#define ND_SADC_EVNT_RESULTDONE     (*(volatile uint32_t*)(ND_SADC_BASE + 0x0000010CUL))       //Base address of event conversion resultdone

#define ND_SADC_RES8BIT      (0x00000000UL)
#define ND_SADC_RES10BIT     (0x00000001UL)
#define ND_SADC_RES12BIT     (0x00000002UL)
#define ND_SADC_RES14BIT     (0x00000003UL)

#define ND_SADC_Over_Bypass               (0UL)
#define ND_SADC_Over_2x                   (1UL)
#define ND_SADC_Over_4x                   (2UL)
#define ND_SADC_Over_8x                   (3UL)
#define ND_SADC_Over_16x                  (4UL)
#define ND_SADC_Over_32x                  (5UL)
#define ND_SADC_Over_64x                  (6UL)
#define ND_SADC_Over_128x                 (7UL)
#define ND_SADC_Over_256x                 (8UL)

#define ND_SADC_SAMPLE_Task                 (0UL)
#define ND_SADC_SAMPLE_CC                   (1UL << 12)

#define ND_SADC_REF_Internal              (0UL)
#define ND_SADC_REF_VDD1_4                (1UL << 12)

#define ND_SADC_RESP_Bypass                 (0UL)
#define ND_SADC_RESP_PullDown               (1UL)
#define ND_SADC_RESP_PullUp                 (2UL)
#define ND_SADC_RESP_VDD1_2                 (3UL)

#define ND_SADC_RESN_Bypass                 (0UL << 4)
#define ND_SADC_RESN_PullDown               (1UL << 4)
#define ND_SADC_RESN_PullUp                 (2UL << 4)
#define ND_SADC_RESN_VDD1_2                 (3UL << 4)

#define ND_SADC_GAIN1_6                     (0UL << 8)
#define ND_SADC_GAIN1_5                     (1UL << 8)
#define ND_SADC_GAIN1_4                     (2UL << 8)
#define ND_SADC_GAIN1_3                     (3UL << 8)
#define ND_SADC_GAIN1_2                     (4UL << 8)
#define ND_SADC_GAIN1                       (5UL << 8)
#define ND_SADC_GAIN2                       (6UL << 8)
#define ND_SADC_GAIN4                       (7UL << 8)

#define ND_SADC_TACQ_3us                    (0UL << 16)
#define ND_SADC_TACQ_5us                    (1UL << 16)
#define ND_SADC_TACQ_10us                   (2UL << 16)
#define ND_SADC_TACQ_15us                   (3UL << 16)
#define ND_SADC_TACQ_20us                   (4UL << 16)
#define ND_SADC_TACQ_40us                   (5UL << 16)

#define ND_SADC_MODE_SE                     (0UL << 20)
#define ND_SADC_MODE_Diff                   (1UL << 20)

#define ND_SADC_En_BURST                    (0UL << 24)
#define ND_SADC_Dis_BURST                   (1UL << 24)

#define ND_SADC_IN_NC                       0UL
#define ND_SADC_IN_Analog0                  1UL
#define ND_SADC_IN_Analog1                  2UL
#define ND_SADC_IN_Analog2                  3UL
#define ND_SADC_IN_Analog3                  4UL
#define ND_SADC_IN_Analog4                  5UL
#define ND_SADC_IN_Analog5                  6UL
#define ND_SADC_IN_Analog6                  7UL
#define ND_SADC_IN_Analog7                  8UL
#define ND_SADC_IN_VDD                      9UL

#define ND_SADC_INT_END                     (1UL << 1)
#define ND_SADC_INT_DONE                    (1UL << 2)
#define ND_SADC_INT_RESULTDONE              (1UL << 3)

extern void adc_init(void);
extern void adc_start(void);
extern void adc_stop(void);
extern uint16_t adc_getdata(void);
extern bool adc_IsBusy(void);
extern void adc_sample(void);
extern void get_adc_array(uint32_t *pBuff);

#endif
