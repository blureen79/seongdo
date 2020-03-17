#include <string.h>
#include <stdlib.h>
#include "nrf_gpio.h"
#include "custom_board.h"
#include "shoponair.h"
#include "ndevice_uart.h"
#include "ndevice_nvmc.h"
#include "ndevice_eeprom.h"
#include "SEGGER_RTT.h"

#if defined(POS_PARSER_ENABLE) 

#define true  1
#define false 0

#define LINE_FEED            0x0A
#define CARIAGE_RETURN       0x0D

#define SIZE_PARSER_BUFFER   128
#define SIZE_PAYMENT_STRING  21
#define SIZE_APPROVAL_STRING 1
#define SIZE_PAYMENT_BUFFER  12
typedef struct{
    unsigned char card;
    unsigned char pay;
    unsigned char paid;
    unsigned char esc_code;
    unsigned char max_len;
    unsigned char min_len;
    unsigned char buffer[SIZE_PARSER_BUFFER + 1];
    unsigned char pay_priority;
    unsigned int  payment;
}POSData_t;


/* Korean : MS959  */
static const char *payment_string[SIZE_PAYMENT_STRING] = {
    "Ã»±¸ÇÕ°è",
    "Ã»±¸±Ý¾×",
    "¹ÞÀ»±Ý¾×",
    "°áÁ¦±Ý¾×",
    "°áÁ¦´ë»ó±Ý¾×",
    "°áÁ¦¿ä±Ý",
    "°áÁ¦ÃÑ¾×",
    "°áÁ¦¾×",
    "¹ÞÀ»µ·",
    "³»½Çµ·",      // 10
    "¿µ¼ö±Ý¾×",
    "ÇÕ°è±Ý¾×",
    "ÃÑÇÕ°è",
    "ÃÑ±Ý¾×",
    "ÃÑ°è",
    "ÃÑ¾×",
    "Total",
    "TOTAL",
    "AMOUNTPAID",
    "ÇÕ°è",       // 20
    "ÁöºÒ",
};

typedef struct{
    unsigned char len;
    unsigned char *pStr;
}TOTAL_STR_t;

#define SIZE_PAYMENT_CHINA   3
/* Chines : GB18030 Table 0x81 ~ 0xFE */
static const unsigned char chinese1_string[] = {0xCF,0xD6, 0xBD,0xF0, 0xD6,0xA7, 0xB8,0xB6, 0x00}; // ?ÑÑò¨Üõ
static const unsigned char chinese2_string[] = {0xAC,0x46, 0xD3,0x8B, 0x00}; // úÞÍª
static const unsigned char chinese3_string[] = {0x9B,0x51, 0x9C,0x67, 0xBD,0xF0, 0xEE,0x7E, 0x00}; // Ì½?ÑÑäþ

/* Japan : SHIFT_JIS */

static TOTAL_STR_t sOtherTotal[3] =
{
    {
        .len = 8,
        .pStr = (unsigned char *)chinese1_string
    },
    {
        .len = 4,
        .pStr = (unsigned char *)chinese2_string
    },
    {
        .len = 8,
        .pStr = (unsigned char *)chinese3_string
    },
};

static POSData_t sPOSData;
static unsigned int pos_buffer_index;

typedef struct{
    unsigned int  sys_option;
    unsigned int  cpk_size;
    unsigned char cpk_string[SIZE_CPK_STR_BUFFER];
    unsigned int  tpk_size;
    unsigned char tpk_string[SIZE_TPK_STR_BUFFER];
}PAYMENT_t;

static PAYMENT_t sPaymentKeyWord;

#define SIZE_OF_CPK_LIST  8
#define SIZE_OF_CPK_STR   13
static unsigned char cpk_str_list[SIZE_OF_CPK_LIST][SIZE_OF_CPK_STR];

static void pos_parser_init_cpk(void)
{
    sPaymentKeyWord.cpk_size = 17;
    memcpy((void *)sPaymentKeyWord.cpk_string,"½ÂÀÎ¹øÈ£,Ãë¼Ò¹øÈ£",17);
    memcpy((void *)&cpk_str_list[0][0], "½ÂÀÎ¹øÈ£", 8);
    memcpy((void *)&cpk_str_list[1][0], "Ãë¼Ò¹øÈ£", 8);
    SEGGER_RTT_printf(0, "\r\n %s : %s", __func__,(void *)&cpk_str_list[0][0]);
    SEGGER_RTT_printf(0, "\r\n %s : %s", __func__,(void *)&cpk_str_list[1][0]);
}

static void pos_parser_init_tpk(void)
{
    sPaymentKeyWord.tpk_size = 0;
    memset((void *)sPaymentKeyWord.tpk_string, 0, SIZE_TPK_STR_BUFFER);
}

void init_pos_parser(void)
{
    unsigned char idx;
    unsigned char max_len,min_len;
    unsigned char str_len;
    sPOSData.card = 0;
    max_len = min_len = strlen(payment_string[0]);
    for(idx = 1; idx < SIZE_PAYMENT_STRING; idx++){
        str_len = (unsigned char)strlen((const char *)payment_string[idx]); 
        if(str_len > max_len)
            max_len = str_len;
        if(str_len < min_len)
            min_len = str_len;
    }
    for(idx = 0; idx < SIZE_PAYMENT_CHINA; idx++){
        str_len = sOtherTotal[idx].len;
        if(str_len > max_len)
            max_len = str_len;
        if(str_len < min_len)
            min_len = str_len;
    }
    sPOSData.max_len = max_len;
    sPOSData.min_len = min_len;
    pos_buffer_index = 0;    
    (void)memset(sPOSData.buffer, 0, SIZE_PARSER_BUFFER + 1);
    memset(sPaymentKeyWord.cpk_string,0, SIZE_CPK_STR_BUFFER);
    memset(cpk_str_list, 0, SIZE_OF_CPK_LIST * SIZE_OF_CPK_STR);
    pos_parser_init_cpk();
    
    SEGGER_RTT_printf(0, "\r\nStart: %s", __func__);
    for(idx = 0; idx < SIZE_OF_CPK_LIST; idx++){
        SEGGER_RTT_printf(0, "\r\n [%d] %s", idx, (char *)&cpk_str_list[idx][0]);
    }
    SEGGER_RTT_printf(0, "\r\nEnd: %s", __func__);
    
    pos_parser_init_tpk();
}

void pos_parser_reset(void)
{
    sPOSData.card = 0;
    sPOSData.pay  = 0;
    sPOSData.paid = 0;
    sPOSData.payment = 0;
    sPOSData.pay_priority = 255;
    pos_buffer_index = 0;
    (void)memset(sPOSData.buffer, 0, SIZE_PARSER_BUFFER + 1);
}

static void pos_parser_total_pay(unsigned char *pPay)
{
    unsigned char idx;
    unsigned char str_len;
    unsigned char data;    
    unsigned char payment[8]; //  9Ãµ¸¸
    unsigned char pay_buf[8];
    unsigned char pay_index;
    str_len = strlen((const char *)pPay);
    if(str_len > 0){
        pay_index = 0;
        (void)memset(payment, 0, 8);
        (void)memset(pay_buf, 0, 8);
        do{
            data = pPay[str_len - 1];
            if(data >= '0' && data <= '9'){
                pay_buf[pay_index++] = data;
            }
            else if(pay_index != 0 && data == ' '){
                break;
            }
            if(pay_index > 7)
                break;
            str_len--;
        }while(str_len > 0);
        if(pay_index > 0){
            str_len = pay_index;
            for(idx = 0; idx < pay_index; idx++){
                payment[idx] = pay_buf[str_len - 1];
                str_len--;
            }
        }
        if(pay_index > 0){
            sPOSData.payment = (unsigned int)atoi((const char *)payment);
            if(sPOSData.pay)
                sPOSData.paid = 1;
            SEGGER_RTT_printf(0,"\r\n Payment: %d",sPOSData.payment);
        }
        else{
            SEGGER_RTT_printf(0,"\r\n NotParser: %s",pPay);
            sPOSData.pay = 0;
        }
    }
}

static unsigned char *pos_parser_strstr(char *dst)
{
    unsigned int index;
    unsigned char *psearch;
    unsigned int str_len,dst_len;
    unsigned int src_idx,dst_idx;
    psearch = (void *)0;
    for(index = 0; index < pos_buffer_index; index++){
        if(sPOSData.buffer[index] == ' ')
            continue;
        if(sPOSData.buffer[index] == dst[0]){
            psearch = &sPOSData.buffer[index];
            str_len = strlen((char *)psearch);
            dst_len = strlen(dst);
            for(src_idx = 0,dst_idx = 0; src_idx < str_len; src_idx++){
                if(psearch[src_idx] == ' ')
                    continue;
                if(psearch[src_idx] != dst[dst_idx])
                  break;
                dst_idx++;
                if(dst_idx == dst_len)
                    break;
            }
            if(dst_idx == dst_len)
                break;
            else
                psearch = (void *)0;
        }
    }
    return psearch;
}

static void pos_parser_payment(unsigned char clean)
{
    unsigned char idx;
    unsigned char str_len;
    unsigned char *pSearched = NULL;  
    if(sPOSData.pay == 0){
        if(sPaymentKeyWord.tpk_size > 0){
            pSearched = pos_parser_strstr((char *)sPaymentKeyWord.tpk_string);
            if(pSearched != NULL){
                str_len = sPaymentKeyWord.tpk_size;
                sPOSData.pay = 1;
                pos_parser_total_pay(pSearched + str_len);
            }
        }
        else{
            if(sPOSData.pay == 0){
                for(idx = 0; idx < SIZE_PAYMENT_STRING; idx++){
                    pSearched = pos_parser_strstr((char *)payment_string[idx]);
                    if(pSearched != NULL){
                        if(sPOSData.pay_priority > idx){
                            sPOSData.pay_priority = idx;
                            str_len = strlen((const char *)payment_string[idx]);
                            if(sPOSData.pay_priority == 0){
                                sPOSData.pay = 1;
                            }
                            pos_parser_total_pay(pSearched + str_len);
                        }
                        break;
                    }
                }
            }
            if(sPOSData.pay == 0){
                for(idx = 0; idx < 3; idx++){
                    pSearched = pos_parser_strstr((char *)sOtherTotal[idx].pStr);
                    if(pSearched != NULL){
                        str_len = sOtherTotal[idx].len;
                        sPOSData.pay = 1;
                        pos_parser_total_pay(pSearched + str_len);
                        break;
                    }
                }
            }
        }
    }
    else if(sPOSData.paid == 0){
        pos_parser_total_pay(sPOSData.buffer);
    }
}

static void pos_parser_approval_number(void)
{
    unsigned char *pSearched;
    unsigned char idx;
    if(sPOSData.card == 0){
        for(idx = 0; idx < SIZE_OF_CPK_LIST; idx++){
          pSearched = pos_parser_strstr((char *)&cpk_str_list[idx][0]);
          if(pSearched != NULL){
              sPOSData.card = 1;
              break;
          }
        }
    }
}

static void pos_parser_shift_buffer(void)
{
    unsigned char idx;
    unsigned int    offset;
    offset = pos_buffer_index - (unsigned int)sPOSData.max_len;
    for(idx = 0; idx < sPOSData.max_len; idx++){
        sPOSData.buffer[idx] = sPOSData.buffer[offset];
        offset++;
    }
    pos_buffer_index = sPOSData.max_len;
}

unsigned char pos_data_is_card(void)
{
    return sPOSData.card;
}

unsigned int pos_parser_get_payment(void)
{
    return sPOSData.payment;
}

void pos_parser_push(unsigned char data)
{
    if(data == 0) 
        return;
    
    sPOSData.buffer[pos_buffer_index++] = data;
    if(data == LINE_FEED && pos_buffer_index < SIZE_PARSER_BUFFER){
        sPOSData.buffer[pos_buffer_index] = 0;
        pos_parser_approval_number();
        pos_parser_payment(true);
        pos_buffer_index = 0;
        return;
    }
    else if(pos_buffer_index >= SIZE_PARSER_BUFFER){
        sPOSData.buffer[pos_buffer_index] = 0;
        pos_parser_approval_number();
        pos_parser_payment(false);
        if(sPOSData.pay == 0 || sPOSData.card == 0)
            pos_parser_shift_buffer();
        else{
            pos_buffer_index = 0;
        }
    }
}

#define SIZE_MINIMUM_CPK 6
void pos_parser_set_CPK(bool new_data, unsigned char *pstr,unsigned char str_len)
{
    unsigned char src_idx,dst_idx;
    unsigned char cpk_list;
    unsigned char cpk_len;
    if(new_data){
        memset(sPaymentKeyWord.cpk_string,0, SIZE_CPK_STR_BUFFER);
        memset(cpk_str_list, 0, SIZE_OF_CPK_LIST * SIZE_OF_CPK_STR);
        pos_parser_init_cpk();
    }
    else{
        if(str_len < 18){
            SEGGER_RTT_printf(0,"\r\n Add CPK None");
            return;
        }
        pstr += 18;
    }
    if(str_len >= (SIZE_USER_STR_BUFFER - 1)){
        SEGGER_RTT_printf(0,"\r\n Add CPK Too Long.");
        return;
    }
    if(str_len < SIZE_MINIMUM_CPK){
        SEGGER_RTT_printf(0,"\r\n Add CPK Too Short.");
        return;
    }
    if(pstr[0] < 0x20 || pstr[0] > 0xFE || pstr[0] == 0x7F){
        SEGGER_RTT_printf(0,"\r\n Add CPK No Valid.");
        return;
    }
    sPaymentKeyWord.cpk_string[17] = ',';
    for(src_idx = 0,dst_idx = 18; src_idx < str_len; src_idx++){
        if(pstr[src_idx] != ' ')
            sPaymentKeyWord.cpk_string[dst_idx++] = pstr[src_idx];
    }
    SEGGER_RTT_printf(0,"\r\n CPK : %s",(char *)sPaymentKeyWord.cpk_string);
    sPaymentKeyWord.cpk_size = dst_idx;
    cpk_list = 0;
    for(src_idx = 0,dst_idx = 0; src_idx < sPaymentKeyWord.cpk_size; src_idx++){
        if(sPaymentKeyWord.cpk_string[src_idx] == ','){
            cpk_list++;
            dst_idx = 0;
            if(cpk_list >= SIZE_OF_CPK_LIST)
                break;
        }
        else{
            cpk_str_list[cpk_list][dst_idx++] = sPaymentKeyWord.cpk_string[src_idx];
            if(dst_idx >= SIZE_OF_CPK_STR)
                dst_idx = SIZE_OF_CPK_STR - 1;
        }
    }
    if(cpk_list > 0){
        for(src_idx = 0; src_idx < cpk_list; src_idx++){
            cpk_len = strlen((char *)&cpk_str_list[src_idx][0]);
            if(cpk_len > sPOSData.max_len)
                sPOSData.max_len = cpk_len;
            if(sPOSData.min_len > cpk_len)
                sPOSData.min_len = cpk_len;
        }
    }
}

unsigned char *pos_parser_get_CPK(void)
{
    return sPaymentKeyWord.cpk_string;
}

#define SIZE_MINIMUM_TPK 4
#define SIZE_MAXIMUM_TPK 12
void pos_parser_set_TPK(unsigned char *pstr,unsigned char str_len)
{
    unsigned char src_idx,dst_idx;
    memset(sPaymentKeyWord.tpk_string,0, SIZE_TPK_STR_BUFFER);
    if(str_len > (SIZE_TPK_STR_BUFFER - 1)){
        pos_parser_init_tpk();
        return;
    }
    if(str_len != strlen((char *)pstr) || str_len == 0 || str_len < SIZE_MINIMUM_TPK
        || str_len > SIZE_MAXIMUM_TPK){
        pos_parser_init_tpk();
        return;
    }
    if(pstr[0] < 0x20 || pstr[0] > 0xFE || pstr[0] == 0x7F){
        pos_parser_init_tpk();
        return;
    }
    for(src_idx = 0,dst_idx = 0; src_idx < str_len; src_idx++){
      if(pstr[src_idx] != ' ')
          sPaymentKeyWord.tpk_string[dst_idx++] = pstr[src_idx];
    }
    sPaymentKeyWord.tpk_size = dst_idx;
    if(sPaymentKeyWord.tpk_size > sPOSData.max_len)
        sPOSData.max_len = sPaymentKeyWord.tpk_size;
    if(sPOSData.min_len > sPaymentKeyWord.tpk_size)
        sPOSData.min_len = sPaymentKeyWord.tpk_size;
}

unsigned char *pos_parser_get_TPK(void)
{
    return sPaymentKeyWord.tpk_string;
}

unsigned char *pos_parser_get_keyword_info(unsigned int *keyword_size)
{
    *keyword_size = sizeof(PAYMENT_t);
    return (unsigned char *)&sPaymentKeyWord;
}


void pos_parser_set_sysinfo(unsigned int options)
{
    sPaymentKeyWord.sys_option = options;
}

void pos_parser_load_cpk_tpk(void)
{
    unsigned int flash_address;
    unsigned int flash_data;
    unsigned char cpk_tpk_str[SIZE_USER_STR_BUFFER];
    unsigned char str_data;
    unsigned int  str_len,str_idx;
    memset(cpk_tpk_str, 0, SIZE_USER_STR_BUFFER);
    flash_address = FLASH_OPTION_ADDR + 4U;
    str_len = nd_nvmc_read(flash_address);
    flash_address += 4U;
    if(str_len > 0 && str_len < SIZE_USER_STR_BUFFER){
        for(str_idx = 0; str_idx < str_len; ){
            flash_data = nd_nvmc_read(flash_address);
            str_data = (uint8_t)(flash_data);
            if(str_data == 0 || str_data == 0xFF)
                break;
            cpk_tpk_str[str_idx++] = str_data;
            str_data = (uint8_t)(flash_data >> 8);
            if(str_data == 0 || str_data == 0xFF)
                break;
            cpk_tpk_str[str_idx++] = str_data;
            str_data = (uint8_t)(flash_data >> 16);
            if(str_data == 0 || str_data == 0xFF)
                break;
            cpk_tpk_str[str_idx++] = str_data;
            str_data = (uint8_t)(flash_data >> 24);
            if(str_data == 0 || str_data == 0xFF)
                break;
            cpk_tpk_str[str_idx++] = str_data;
            flash_address += 4U;
        }
        pos_parser_set_CPK(false, cpk_tpk_str,str_len);
    }
    memset(cpk_tpk_str, 0, SIZE_USER_STR_BUFFER);
    flash_address = FLASH_OPTION_ADDR + 8U + SIZE_CPK_STR_BUFFER;
    str_len = nd_nvmc_read(flash_address);
    flash_address += 4U;
    if(str_len > 0 && str_len < SIZE_TPK_STR_BUFFER){
        for(str_idx = 0; str_idx < str_len; ){
            flash_data = nd_nvmc_read(flash_address);
            str_data = (uint8_t)(flash_data);
            if(str_data == 0 || str_data == 0xFF)
                break;
            cpk_tpk_str[str_idx++] = str_data;
            str_data = (uint8_t)(flash_data >> 8);
            if(str_data == 0 || str_data == 0xFF)
                break;
            cpk_tpk_str[str_idx++] = str_data;
            str_data = (uint8_t)(flash_data >> 16);
            if(str_data == 0 || str_data == 0xFF)
                break;
            cpk_tpk_str[str_idx++] = str_data;
            str_data = (uint8_t)(flash_data >> 24);
            if(str_data == 0 || str_data == 0xFF)
                break;
            cpk_tpk_str[str_idx++] = str_data;
            flash_address += 4U;
        }
        pos_parser_set_TPK(cpk_tpk_str,str_len);
    }
}
#endif

