#include "custom_board.h"
#include "app_uart.h"
#include "nrf_gpio.h"
#include "nrf_delay.h"
#include "ndevice_eeprom.h"
#include "ndevice_uart.h"

#include "shoponair.h"
#include "process_data.h"
#include "func.h"

#include "SEGGER_RTT.h"

#define MAX_PRINT_TYPES      2

#define VOICE_INFO_BAUDRATE  79
#define VOICE_INFO_ERROR     78

static unsigned char communication_dir = 0;
static void auto_baud_rate_switch_direct(void)
{
    nrf_gpio_pin_set(RS232_IN1);
    nrf_gpio_pin_clear(POS_DIR);
    communication_dir = 1;
    nrf_delay_ms(100); // Stable for Relay
}

static void auto_baud_rate_switch_cross(void)
{
    nrf_gpio_pin_clear(RS232_IN1);
    nrf_gpio_pin_set(POS_DIR);
    communication_dir = 2;
    nrf_delay_ms(100); // Stable for Relay
}

static void auto_baud_rate_switch_to_print(void)
{
  NRF_UART0->TASKS_STARTRX = 0;
  
  NRF_UART0->PSELRXD = PRN_RX_PIN_NUMBER; 
  nrf_gpio_pin_set(RX_PIN_NUMBER);
  nrf_gpio_pin_set(PRN_RX_PIN_NUMBER);
    
  NRF_UART0->TASKS_STARTRX = 1;
}

static void auto_baud_rate_switch_to_pos(void)
{
  NRF_UART0->TASKS_STARTRX = 0;
  
  NRF_UART0->PSELRXD = RX_PIN_NUMBER; 
  nrf_gpio_pin_set(RX_PIN_NUMBER);
  nrf_gpio_pin_set(PRN_RX_PIN_NUMBER);
    
  NRF_UART0->TASKS_STARTRX = 1;
}

static void qr_read_handler(void)
{
}

static void auto_baudrate_print(void)
{
  char tempbuff[35]; 		//0212 KSD 32 -> 35
  
  NRF_WDT->RR[0] = WDT_RR_RR_Reload;
  sprintf(tempbuff, "ShopOnAir POS Connector\n");
  uart_putstring((uint8_t*)tempbuff);
  sprintf(tempbuff, "========================\n");
  uart_putstring((uint8_t*)tempbuff);
  sprintf(tempbuff, "F/W Ver: %s %s\n", FIRMWARE_NAME, __DATE__);
  uart_putstring((uint8_t*)tempbuff);
  switch(PoscvtCfg_g.mode){
      case CFG_MODE_CAT:
          sprintf(tempbuff, "MODE : CAT\n");
          break;
      case CFG_MODE_KEYPAD:
          sprintf(tempbuff, "MODE : KEYPAD\n");
          break;
      case CFG_MODE_POS:
          sprintf(tempbuff, "MODE : POS\n");
          break;
      default:
          sprintf(tempbuff, "MODE : POS\n");
          break;
  }
  uart_putstring((uint8_t*)tempbuff);
  sprintf(tempbuff, "LANG : 가나다라마바\n");
    
  switch(PoscvtCfg_g.print_type){
      case CFG_PRINT_A:
        sprintf(tempbuff, "TYPE : PRINTER A\n");
        break;
      case CFG_PRINT_B:
        sprintf(tempbuff, "TYPE : PRINTER B\n");
        break;
      case CFG_PRINT_C:
        sprintf(tempbuff, "TYPE : PRINTER C\n");
        break;
      default:
        sprintf(tempbuff, "TYPE : PRINTER A\n");
        break;
  }
  uart_putstring((uint8_t*)tempbuff);
  
  switch(PoscvtCfg_g.baudrate)
  {
  case CFG_BAUD_9600:
    sprintf(tempbuff, "BAUDRATE : 9600\n");  
    break;
  case CFG_BAUD_19200:
    sprintf(tempbuff, "BAUDRATE : 19200\n");  
    break;
  case CFG_BAUD_38400:
    sprintf(tempbuff, "BAUDRATE : 38400\n");  
    break;
  case CFG_BAUD_57600:
    sprintf(tempbuff, "BAUDRATE : 57600\n");  
    break;
  case CFG_BAUD_115200:
    sprintf(tempbuff, "BAUDRATE : 115200\n");  
    break;
  default:
    sprintf(tempbuff, "BAUDRATE : 9600\n");  
    break;
  }
  uart_putstring((uint8_t*)tempbuff);
  sprintf(tempbuff, "CARD CHK:%s\n",pos_parser_get_CPK());
  uart_putstring((uint8_t*)tempbuff);
  sprintf(tempbuff, "TOTAL AMT:%s\n",pos_parser_get_TPK());
  uart_putstring((uint8_t*)tempbuff);
  sprintf(tempbuff, "VOLUME : %d\n", ((16 - PoscvtCfg_g.volume) / 4) + 1);
  uart_putstring((uint8_t*)tempbuff);
  
  sprintf(tempbuff, "QR CODE : ");
  uart_putstring((uint8_t*)tempbuff);
  {
      uint32_t qr_header,qr_tail;
      uint32_t qr_read_len;
      uint32_t qr_q_index;
      qr_header = bleQueue_g.QRData.idx_hdr;
      qr_tail   = bleQueue_g.QRData.idx_tail;
      while(qr_header > qr_tail){
        qr_read_len = qr_header - qr_tail;    
        if(qr_read_len >= 30){
            qr_read_len = 33; // qr_read_len + 3
            eep_read_array(EEP_QRDATA_ADDR + qr_tail, (uint8_t *)tempbuff, qr_read_len, qr_read_handler);
        }
        else
            eep_read_array(EEP_QRDATA_ADDR + qr_tail, (uint8_t *)tempbuff, qr_read_len, qr_read_handler);
        while(IsSPIBusy()) 
        {
            NRF_WDT->RR[0] = WDT_RR_RR_Reload;            //clear watchdog
        }     
        qr_q_index = 0;
        while(qr_read_len > qr_q_index){
            uart_put(tempbuff[3 + qr_q_index++]); 				//0212 KSD
            NRF_WDT->RR[0] = WDT_RR_RR_Reload;            //clear watchdog
        }
        qr_tail += qr_read_len;
    }
  }
  sprintf(tempbuff, "\n\n\n\n\n%c%c", 0x1B, 0x69);
  uart_putstring((uint8_t*)tempbuff);  
  NRF_WDT->RR[0] = WDT_RR_RR_Reload;
}

static void auto_baud_rate_write_option(void)
{
    volatile uint32_t* pFlagAddress = (uint32_t*)(FLASH_OPTION_ADDR);
    uint32_t flashData = 0;
    uint32_t lenToWrite = 0;
    uint32_t const *pOptionBuf;
    flashData = 0;
    flashData |= (uint32_t)PoscvtCfg_g.print_type;
    flashData <<= 8;        
    flashData |= (uint32_t)PoscvtCfg_g.volume;
    flashData <<= 8;      
    flashData |= (uint32_t)PoscvtCfg_g.baudrate;
    flashData <<= 8;
    flashData |= (uint32_t)PoscvtCfg_g.mode;

    pos_parser_set_sysinfo(flashData);
    pOptionBuf = (uint32_t const *)pos_parser_get_keyword_info(&lenToWrite);
    nd_write_flash((uint32_t*)pFlagAddress, pOptionBuf, lenToWrite);
    nrf_delay_ms(1 + lenToWrite/4); // 336us/word
}

static void auto_baud_rate_play_voice(unsigned char voice_index)
{
    unsigned int timeout;
    SoundPlay(voice_index);
    for(timeout = 0xFFFF; timeout>0; --timeout)
    {
      if(nrf_gpio_pin_read(DEF_AUDIO_BUSY) == 0) 
          break;    
      SEGGER_RTT_printf(0, ".");
      nrf_delay_ms(10);
      NRF_WDT->RR[0] = WDT_RR_RR_Reload;
    }
}

static const unsigned char print_check_status_command[3] = { 0x10, 0x04, 0x01};
static void auto_baud_rate_proc(void)
{    
    unsigned char rx_data;    
    unsigned char baud_rate;
    unsigned char process_status;
    unsigned char baud_rate_index;
    unsigned char print_type;
    unsigned int wait_time = get_tickcount();
    for(print_type = CFG_PRINT_A; print_type < CFG_PRINT_C; print_type++){
        baud_rate = 0;
        process_status = 0;
        baud_rate_index = 255;             
        do{
            if(process_status == 0){
                switch(baud_rate){
                case CFG_BAUD_9600:
                    uart_setBaudrate(9600);
                    break;
                case CFG_BAUD_19200:
                    uart_setBaudrate(19200);
                    break;
                case CFG_BAUD_38400:
                    uart_setBaudrate(38400);
                    break;
                case CFG_BAUD_57600:
                    uart_setBaudrate(57600);
                    break;
                case CFG_BAUD_115200:
                    uart_setBaudrate(115200);
                    break;
                default:
                    uart_setBaudrate(9600);
                    baud_rate = 1;
                    break;
                }
                process_status = 1;
                uart_put(print_check_status_command[0]);
                uart_put(print_check_status_command[1]);
                uart_put(print_check_status_command[2]);
                wait_time = get_tickcount();
            }
            else if(process_status == 1){
                rx_data = 0;
                if(uart_get_RXD(&rx_data)){
                    if(rx_data == 0x16){ //POS Print Response
                        baud_rate_index = baud_rate;
                        SEGGER_RTT_printf(0, "\n[%d]Rx : 0x%x", baud_rate, rx_data);
                        baud_rate = CFG_BAUD_115200 + 1;
                    }
                }
                if((get_tickcount() - wait_time) >= 10){ // 100ms
                    wait_time = get_tickcount();
                    process_status = 0;
                    baud_rate++;
                }
            }
            else
                process_status = 0;
            NRF_WDT->RR[0] = WDT_RR_RR_Reload;
        }while(baud_rate <= CFG_BAUD_115200);
        if(baud_rate_index != 255){
            if(PoscvtCfg_g.baudrate != baud_rate_index || PoscvtCfg_g.print_type != print_type){
                PoscvtCfg_g.baudrate = baud_rate_index;
                PoscvtCfg_g.print_type = print_type;
                nd_erase_flash_opt(1); 
                nrf_delay_ms(100); // Wait for erase                
                auto_baud_rate_write_option();  
                nrf_delay_ms(10);  // Wait for write
                communication_dir = 255;
            }
            auto_baudrate_print();
            return;
        }
        else{ // Not Connected
            if(communication_dir == 1){
                auto_baud_rate_switch_cross(); // Swich to Print B
            }
            else{
                auto_baud_rate_play_voice(VOICE_INFO_BAUDRATE); // Baud Rate
                auto_baud_rate_play_voice(VOICE_INFO_ERROR); // Cancel
            }
        }
    }    
}

bool auto_baud_rate_run(void)
{
    unsigned char status = 0;
    unsigned char button_bounce = 0;
    unsigned int uart_wait_time = get_tickcount();

    if(nrf_gpio_pin_read(DEF_GPIO_BUTTON) == 1)
       return false;
    do{
       if(status == 0){
           if(nrf_gpio_pin_read(DEF_GPIO_BUTTON) == 0){  // Select Baud Rate             
               button_bounce++;               
           }
           else{
               button_bounce = 0;
           }
           if(button_bounce > 10){
               button_bounce = 0;
               status = 1;
           }
       }
       else if(status == 1){
           auto_baud_rate_play_voice(VOICE_INFO_BAUDRATE);
           status = 2;
       }
       else if(status == 2){
           if(nrf_gpio_pin_read(DEF_GPIO_BUTTON) == 1){ // Check Button Release
               button_bounce++;
           }
           else{
               button_bounce = 0;
           }
           if(button_bounce > 10){
               auto_baud_rate_switch_to_pos();   
               button_bounce = 0;
               status = 3;
           }           
       }
       else if(status == 3){
           if(nrf_gpio_pin_read(DEF_GPIO_BUTTON) == 0){ // Check Button Press
               button_bounce++;
           }
           else{
               button_bounce = 0;
           }
           if(button_bounce > 10){
               auto_baud_rate_switch_direct();
               button_bounce = 0;
               status = 4;
           }           
       }
       else if(status == 4){
           if(nrf_gpio_pin_read(DEF_GPIO_BUTTON) == 1){ // Check Button Release
               button_bounce++;
           }
           else{
               button_bounce = 0;
           }
           if(button_bounce > 10){
               button_bounce = 0;
               status = 5;
           }
       }
       else if(status == 5){
           auto_baud_rate_switch_to_print(); // Switch to Print A
           auto_baud_rate_proc();
           break;
       }
       NRF_WDT->RR[0] = WDT_RR_RR_Reload;
    }while((get_tickcount() - uart_wait_time)  < 1000);
    auto_baud_rate_switch_to_pos();  
    if(communication_dir == 255)
        return true;
    else
        return false;
}


