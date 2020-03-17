
#ifndef __NDEVICE_SHOPONAIR
#define __NDEVICE_SHOPONAIR

#include <stdint.h>

#define DEF_SOUND_DELAY                         1

#define MAX_QRCODE_COUNT                        5
            
#define EEP_QRINFO_ADDR                         6
#define EEP_QRDATA_ADDR                         2048
//#define EEP_QRDATA_LIMIT                        EEP_QRDATA_ADDR
#define EEP_QRDATA_LIMIT                        512
#define EEP_REVERSE_ADDR                        4
#define EEP_REVDATA_ADDR                        10
#define EEP_RECEIPT_ADDR                        (1024 * MAX_QRCODE_COUNT + EEP_QRDATA_ADDR + 100)                   //100 is margin
//#define EEP_ADDR_LIMIT                          (0x8000 - EEP_RECEIPT_ADDR - 100)                                         //0x8000 is 256kbit, 100 is margin
#define EEP_ADDR_LIMIT                          (0x8000 - 100)                                         //0x8000 is 256kbit, 100 is margin

//#define MAXSIZE_BASE64                          30              //30 bytes will be 40 bytes for base54
//#define MAXSIZE_BASE64                          45              //45 bytes will be 60 bytes for base54
#define MAXSIZE_BASE64                          90              //45 bytes will be 60 bytes for base54

#if 0
#define BEEP_SOUND                              {SEGGER_RTT_printf(0,"!");}
#define ERROR_SOUND                             {SEGGER_RTT_printf(0,"@");}
#define RECEIPT_SOUND                           {SEGGER_RTT_printf(0,"*");}
#define TAGGING_SOUND                           {SEGGER_RTT_printf(0,"^");}
#define WORKEND_SOUND                           {SEGGER_RTT_printf(0,"&");}
#else
#define BEEP_SOUND                              {SoundPlay(27);}
#define ERROR_SOUND                             {SoundPlay(72);} // 삐 삐 삐 3개를 빠른 간격으로
#define ERROR_SOUND2                            {SoundPlay(72);} // 삐 삐 삐 3개를 느린 간격으로
#define RECEIPT_SOUND                           {SoundPlay(75);} // Windows 사용자 계정 컨트롤.wav
#define TAGGING_SOUND                           {SoundPlay(70);} // Windows Notify System Generic.wav
#define WORKEND_SOUND                           {SoundPlay(75);}
#endif

extern void totalHap(uint8_t* total) ;
extern void SoundVol(uint8_t value);
extern void SoundPU(void);
extern void SoundPlay(uint16_t data);


#define SOUND_ENABLE_BUFFER

#ifdef SOUND_ENABLE_BUFFER
extern void init_play_sound(void);
extern void play_sound_push(unsigned char data);
extern void play_sound_set_total_value(unsigned char *pvalue , _Bool lsat_sound);
extern unsigned char play_sound(void);
#undef BEEP_SOUND
#undef ERROR_SOUND
#undef ERROR_SOUND2
#undef RECEIPT_SOUND
#undef TAGGING_SOUND
#undef WORKEND_SOUND

#define BEEP_SOUND      {play_sound_push(27); }
#define ERROR_SOUND     {play_sound_push(72); }
#define ERROR_SOUND2    {play_sound_push(72); }
#define RECEIPT_SOUND   {play_sound_push(75); } // X_M6_Windows
#define TAGGING_SOUND   {play_sound_push(70); } // X_M1_Windows Notify System Generic
#define WORKEND_SOUND   {play_sound_push(75); }

#define CERTIFIED_ERROR_SOUND 	{play_sound_push(90); } // "비정상접속입니다"
#define POS_MODE_SOUND 		{play_sound_push(80); } // "포스모드"
#define CARD_MODE_SOUND 	{play_sound_push(81); } // "카드승인모드"
#define KEYPAD_MODE_SOUND       {play_sound_push(82); } // "키패드모드"
#define NORMAL_SOUND 	        {play_sound_push(85); } // "정상처리되었습니다"
#define FORCE_DISCONN_SOUND 	{play_sound_push(89); } // "연결을 중단합니다"
#define CANCEL_SOUND            {play_sound_push(78); } // "취소되었습니다"
#define NO_RECEIPT_SOUND        {play_sound_push(86); } // "입력한 다음에 태깅하세요"
#define PRINT_PASS_SOUND        {play_sound_push(88); } // "인쇄를 생략합니다"
#endif

#define POS_PARSER_ENABLE

#ifdef POS_PARSER_ENABLE
#define SIZE_CPK_STR_BUFFER  108
#define SIZE_TPK_STR_BUFFER  13
#define SIZE_USER_STR_BUFFER 108

//#define PRINT_RESULT
extern void init_pos_parser(void);
extern void pos_parser_push(unsigned char data);
extern unsigned char pos_data_is_card(void);
extern void pos_parser_reset(void);
extern unsigned int pos_parser_get_payment(void);

extern void pos_parser_load_cpk_tpk(void);

#define ENABLE_REMOVE_SPACE
extern void pos_parser_set_CPK(bool new_data, unsigned char *pbuff,unsigned char str_len);
extern unsigned char *pos_parser_get_CPK(void);
extern void pos_parser_set_TPK(unsigned char *pbuff,unsigned char str_len);
extern unsigned char *pos_parser_get_TPK(void);

extern void pos_parser_set_sysinfo(unsigned int options);
extern unsigned char *pos_parser_get_keyword_info(unsigned int *keyword_size);
extern void pos_parser_set_baudrate(unsigned int reg_value);
extern unsigned int pos_parser_get_baudrate(void);
#endif

#define FIRMWARE_NAME   "SOA_PC_001"

#define ENABLE_SOA_DFU  1
#ifdef ENABLE_SOA_DFU
#include "sdk_macros.h"
#define BOOTLOADER_DFU_GPREGRET                 (0xB0)      /**< Magic pattern written to GPREGRET register to signal between main app and DFU. The 3 lower bits are assumed to be used for signalling purposes.*/
#define BOOTLOADER_DFU_START_BIT_MASK           (0x01)      /**< Bit mask to signal from main application to enter DFU mode using a buttonless service. */
#define BOOTLOADER_DFU_START    (BOOTLOADER_DFU_GPREGRET | BOOTLOADER_DFU_START_BIT_MASK)
extern uint32_t func_enter_bootloader(void);
#endif

#endif
