#include "string.h"
#include "nrf_wdt.h"
#include "nrfx_wdt.h"
#include "process_data.h"
#include "ndevice_rf.h"
#include "custom_board.h"
#include "ndevice_timer.h"
#include "ndevice_nvmc.h"
#include "ndevice_eeprom.h"
#include "ndevice_uart.h"
#include "shoponair.h"
#include "func.h"
#include "SEGGER_RTT.h"

#define TRUE                                    1
#define FALSE                                   0

#define ND_W_QUEUE_SIZE                         10

static ble_nus_t* pNus_g;
static uint16_t handle_g;

stBLEQueue bleQueue_g;
static void receipt_handler(void);
static uint16_t ble_send_packet_count = 0;
static uint8_t reverseDataCount_g = 0;
static uint8_t targetDataCount_g = 0;
bool last_sound;			// 1:결제되었습니다 , 0 : 무음
bool no_receipt_flag; // 1:입력한 다음에 태깅하세요
bool prf_cancel_flag; // 1:취소되었습니다
bool print_pass_flag; // 1:인쇄를생략합니다

void receipt_handler(void)
{

}

uint16_t process_is_there_packet(void)
{
    return ble_send_packet_count;
}

/*
* @brief : 프로세스에서 사용되는 변수 및 버퍼를 초기화
*/
void init_process(void)
{
  bleQueue_g.data_idx_hdr = 0;
  bleQueue_g.data_idx_tail = 0;
  bleQueue_g.QRData.idx_hdr = 0;
  bleQueue_g.QRData.idx_tail = 0;
  bleQueue_g.state = QUEUE_STANDBY;
  bleQueue_g.IsBusy = false;
  bleQueue_g.base64Len = 0;
  bleQueue_g.sentSuccess = false;
  bleQueue_g.CAT2Len = 0;
  last_sound = true;									//BLE Auto cut off 기능추가 2020.02.07 ksd
  no_receipt_flag = false;
  prf_cancel_flag = false;
  print_pass_flag = false;
  PoscvtCfg_g.ble_tag_connection = false;

  memset(bleQueue_g.tempCAT2Buff, 0x00, 20);
  memset(bleQueue_g.CAT2Buff, 0x00, 20);
}

static bool process_over_time(uint32_t ref_times, uint32_t diff_times)
{
    uint32_t curr_time = get_tickcount();
    if(curr_time >= ref_times){
        if((curr_time - ref_times) > diff_times)
            return true;
    }
    else{
        curr_time = 0xFFFFFFFF - curr_time;
        if((curr_time - ref_times) > diff_times)
            return true;
    }
    return false;
}

/*
* @bire ; packet parser
*/
void gen_qode_packet(ble_nus_t* m_nus, uint8_t *pBuff, uint16_t len, uint16_t handle)
{
    uint32_t wait_time;
    uint16_t length = len;
    ret_code_t err_code = NRF_SUCCESS;
    if(IsAdvertised() != false){
        return;
    }
    if(IsConnected() != true){
        return;
    }
    //if(ble_get_handle_state() == BLE_CONN_HANDLE_INVALID) return;
    wait_time = get_tickcount();
    do{
        err_code = ble_nus_data_send(m_nus, pBuff, &length, handle);
        if ((err_code != NRF_ERROR_INVALID_STATE) &&
            (err_code != NRF_ERROR_RESOURCES) &&
            (err_code != NRF_ERROR_NOT_FOUND))
        {
            APP_ERROR_CHECK(err_code);
        }
        if(process_over_time(wait_time, 20))
            break;
    }while(err_code == NRF_ERROR_RESOURCES);
}

/*
* @ brief : BLE관련 포인터
*/
void set_nus_ptr(ble_nus_t* pNus)
{
  pNus_g = pNus;
}


/*
*@ brief : 큐된 데이터를 처리하는 프로세스
*/
bool send_queuedQRData(void)
{
  uint16_t LenToSend = 0;
  uint16_t available = 0;
  ret_code_t err_code;
  bool ret = false;
  uint8_t orgData[MAXSIZE_BASE64 + 50];

  if(bleQueue_g.IsBusy == true){
      return ret;
  }
  switch(bleQueue_g.state)
  {
  case QUEUE_STANDBY:
    if(bleQueue_g.QRData.idx_hdr != 0)
    {
      bleQueue_g.state = QUEUE_QRCODE;
      
      bleQueue_g.QRData.idx_tail = 0;
    }
    else
    {
      bleQueue_g.state = QUEUE_QRCODE_END;
    }
    break;

  case QUEUE_QRCODE:
    resetRemainConnect();

    if(bleQueue_g.QRData.idx_tail >= bleQueue_g.QRData.idx_hdr)
    {
      bleQueue_g.state = QUEUE_QRCODE_END;
    }
    else if(bleQueue_g.QRData.idx_tail != bleQueue_g.QRData.idx_hdr)
    {
      if((bleQueue_g.QRData.idx_hdr - bleQueue_g.QRData.idx_tail) > MAXSIZE_BASE64)
      {
        LenToSend = MAXSIZE_BASE64;
        eep_read_array(EEP_QRDATA_ADDR + bleQueue_g.QRData.idx_tail, orgData, LenToSend, receipt_handler);
        while(IsSPIBusy()) 
        {
        }     
        memcpy(bleQueue_g.base64Buff, orgData + 3, LenToSend);
        bleQueue_g.base64Len = LenToSend;    
      }
      else
      {
        LenToSend = bleQueue_g.QRData.idx_hdr - bleQueue_g.QRData.idx_tail;
        eep_read_array(EEP_QRDATA_ADDR + bleQueue_g.QRData.idx_tail, orgData, LenToSend, receipt_handler);
        while(IsSPIBusy()) 
        {
        }                  
        memcpy(bleQueue_g.base64Buff, orgData + 3, LenToSend);
        bleQueue_g.base64Len = LenToSend;
        bleQueue_g.state = QUEUE_QRCODE_END;
      }
    }
    break;
    
  case QUEUE_QRCODE_END:
      bleQueue_g.state = QUEUE_RECEIPT;
      send_endQR();
      LenToSend = 0;
    break;
  case QUEUE_RECEIPT:
      if(PoscvtCfg_g.IsReceiptdata == false){
        bleQueue_g.state = QUEUE_STANDBY;
        send_modeSum(false);
      }
      else{
        send_modeSum(true);        
      }
      ret = true;
      break;
  default:
    bleQueue_g.state = QUEUE_STANDBY;
    break;    
  }
  if(bleQueue_g.base64Len)                   //first packet
  {  
    unsigned int wait_time = 0;        
    wait_time = get_tickcount();
    do{
        available = bleQueue_g.base64Len;
        err_code = ble_nus_data_send(pNus_g, bleQueue_g.base64Buff, &available, handle_g);
        if ((err_code != NRF_ERROR_INVALID_STATE) &&
            (err_code != NRF_ERROR_RESOURCES) &&
            (err_code != NRF_ERROR_NOT_FOUND))
        {
            APP_ERROR_CHECK(err_code);
        }
                        
        if(err_code == NRF_SUCCESS)
        {
            if(bleQueue_g.state == QUEUE_RECEIPT) bleQueue_g.data_idx_tail += ((available * 6)>>3);
            else bleQueue_g.QRData.idx_tail += available;

            bleQueue_g.base64Len -= available;
            bleQueue_g.IsBusy = true;      
            break;
        }
        else
        {
            SEGGER_RTT_printf(0,"\n %s : 0x%x", __func__, err_code); 
        }
        if(process_over_time(wait_time,20)){ // 200ms
          break;
        }
    }while(err_code == NRF_ERROR_RESOURCES);
  }  
  
  return ret;
}

bool send_receiptData(void)
{
    uint16_t LenToSend = 0;
    uint16_t available = 0;
    ret_code_t err_code;
    bool ret = false;
    uint8_t orgData[MAXSIZE_BASE64 + 50];
    if(bleQueue_g.IsBusy == true){ 
        return ret;
    }
    switch(bleQueue_g.state){
        case QUEUE_RECEIPT:
            resetRemainConnect();
            if(bleQueue_g.data_idx_tail >= bleQueue_g.data_idx_hdr){
                bleQueue_g.state = QUEUE_RECEIPT_END;
            }
            else if(bleQueue_g.data_idx_tail != bleQueue_g.data_idx_hdr){
                if((bleQueue_g.data_idx_hdr - bleQueue_g.data_idx_tail) >= MAXSIZE_BASE64){
                    LenToSend = MAXSIZE_BASE64;
                    eep_read_array(EEP_RECEIPT_ADDR + bleQueue_g.data_idx_tail, orgData, LenToSend, receipt_handler);
                    while(IsSPIBusy());
                    if(setBase64Buffer(orgData + 3, LenToSend) != 0){
                       //SEGGER_RTT_printf(0, "Failed base64");
                    }    
                }
                else{
                    LenToSend = bleQueue_g.data_idx_hdr - bleQueue_g.data_idx_tail;
                    eep_read_array(EEP_RECEIPT_ADDR + bleQueue_g.data_idx_tail, orgData, LenToSend, receipt_handler);
                    while(IsSPIBusy());
                    if(setBase64Buffer(orgData + 3, LenToSend) != 0){
                        //SEGGER_RTT_printf(0, "Failed base64");
                    }        
                    bleQueue_g.state = QUEUE_RECEIPT_END;  
                }
            }
            break;
        case QUEUE_RECEIPT_END:
            PoscvtCfg_g.sendPRS = true;
            bleQueue_g.state = QUEUE_STANDBY;
            LenToSend = 0;
            setSoundTotal(pos_parser_get_payment());
						
						SEGGER_RTT_printf(0, "\n%d Parser Payment %d ",get_tickcount(),pos_parser_get_payment());
						// 7DIG SEGMENT DISPLAY
				
            set_SysEvent(SYSEVT_PLAY_SOUND);
            ret = true;
            break;
        default:
            clear_receipt();
            bleQueue_g.base64Len = 0;
            ret = true;
            break;
    }
    if(bleQueue_g.base64Len)                   //first packet
    {  
        unsigned int wait_time = 0;        
        wait_time = get_tickcount();
        do{
            available = bleQueue_g.base64Len;
            err_code = ble_nus_data_send(pNus_g, bleQueue_g.base64Buff, &available, handle_g);
            if ((err_code != NRF_ERROR_INVALID_STATE) &&
                (err_code != NRF_ERROR_RESOURCES) &&
                (err_code != NRF_ERROR_NOT_FOUND))
            {
                APP_ERROR_CHECK(err_code);
            }
            if(err_code == NRF_SUCCESS)
            {
                if(bleQueue_g.state == QUEUE_RECEIPT) 
                    bleQueue_g.data_idx_tail += ((available * 6)>>3);
                else 
                    bleQueue_g.QRData.idx_tail += available;
                bleQueue_g.base64Len -= available;
                ble_send_packet_count++;
                break;
            }
            if(process_over_time(wait_time,20)){ // 200ms
                break;
            }
        }while(err_code == NRF_ERROR_RESOURCES);
    }  
    return ret;
}

void print_left_align(void)
{
static const unsigned char print_cmd[6] = { 0x1B, 0x70, 0x30,  //Generate pulse
																							0x1B, 0x61, 0x30}; //Left Align Cmd
	
    for(int i=0 ; i < 6 ; i++)
    {
       uart_put(print_cmd[i]);
    }
}

static void qr_read_handler(void)
{
}
/*
*@ brief: 프린트 초기화
*/
void init_parameter(void)
{
  uint16_t eep_addr = EEP_RECEIPT_ADDR;
  uint8_t tempBuff[10];
	char tempbuff[35]; 		//0212 KSD 32 -> 35

//초기 상태 인쇄 출력시 프린터 모드를 먼저 선택 후 출력필요 20200208 ksd modify
  set_print_mode(PoscvtCfg_g.print_type); 

//200208.ksd.modify Print set position alignment : Left
  print_left_align();
#if 1
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
  uart_putstring((uint8_t*)tempbuff);
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
  sprintf(tempbuff, "QR CODE : "); //0211 KSD
	SEGGER_RTT_printf(0,"%s",(uint8_t*)tempbuff);
	
  uart_putstring((uint8_t*)tempbuff);
  {
      uint32_t qr_header,qr_tail;
      uint32_t qr_read_len;
      uint32_t qr_q_index;
      qr_header = bleQueue_g.QRData.idx_hdr;
      qr_tail   = bleQueue_g.QRData.idx_tail;
      while(qr_header > qr_tail){
        qr_read_len = qr_header - qr_tail;
				//SEGGER_RTT_printf(0,"\n %d qr_read_len: %d , tail: %d ,",get_tickcount(),qr_read_len,qr_tail);
				
        if(qr_read_len >= 30){
            qr_read_len = 33; // qr_read_len + 3
            eep_read_array(EEP_QRDATA_ADDR + qr_tail, (uint8_t *)tempbuff, qr_read_len, qr_read_handler);
        }
        else{
            eep_read_array(EEP_QRDATA_ADDR + qr_tail, (uint8_t *)tempbuff, qr_read_len, qr_read_handler);
				}						
        while(IsSPIBusy()) 
        {
            NRF_WDT->RR[0] = WDT_RR_RR_Reload;            //clear watchdog
        }     
        qr_q_index = 0;
        while(qr_read_len > qr_q_index){
						uart_put(tempbuff[3 + qr_q_index++]); 				//0212 KSD
					  //SEGGER_RTT_printf(0,"%02X ",tempbuff[qr_q_index + 3 - 1]);
            NRF_WDT->RR[0] = WDT_RR_RR_Reload;            //clear watchdog
        }
        qr_tail += qr_read_len;
    }
  }
	
  sprintf(tempbuff, "\n\n\n\n\n\n\n\n\n\n%c%c", 0x1B, 0x69);
  uart_putstring((uint8_t*)tempbuff);  
  NRF_WDT->RR[0] = WDT_RR_RR_Reload;
#else	
  uart_put('\n');
  uart_put('\n');
  uart_put('\n');
  SEGGER_RTT_printf(0,"\r\n %s BaudRate: 0x%x", __func__, PoscvtCfg_g.baudrate);
  set_print_baud_rate(PoscvtCfg_g.baudrate);
  print_system_prameter(false);
#endif
  
  eep_addr = 0;                                     //10 is start address
  tempBuff[0] = (uint8_t)(eep_addr>>8);
  tempBuff[1] = (uint8_t)eep_addr;
  eep_write_array(0, tempBuff, 2);                //write length of data at 0
  while(IsSPIBusy()) 
  {
    //NRF_WDT->RR[0] = WDT_RR_RR_Reload;            //clear watchdog
  }
  SoundVol(PoscvtCfg_g.volume);  
  play_sound_mode(PoscvtCfg_g.mode);
  //set_print_mode(PoscvtCfg_g.print_type); //2020.02.07 modify
}

static void print_compiled_date(bool skip_time)
{
    char date[30] = __DATE__;  // 
    char *ptr = strtok(date, " "); 
    char month[5];
    char day[5];
    char year[5];
    char new_format[30];
    if(ptr != NULL)
        sprintf(month, "%s", ptr);
    ptr = strtok(NULL," ");
    if(ptr != NULL)
        sprintf(day, "%s", ptr);
    ptr = strtok(NULL," ");
    if(ptr != NULL)
        sprintf(year, "%s", ptr);
    if(skip_time){
        sprintf(new_format,"F/W Ver : %s %s %s\n",year, month, day);
    }
    else{
        sprintf(new_format,"F/W Ver : %s %s %s %s\n",year, month, day, __TIME__);
    }
    uart_putstring((uint8_t*)new_format);
}

/*
*@ brief: 샘플데이터 프린트
*/
void print_system_prameter(bool change)
{
  char tempBuff[30];
  char posmode[30];
  uint32_t baudRate = 0;

  switch(PoscvtCfg_g.mode)
  {
  case CFG_MODE_CAT:
    sprintf(posmode, "CAT MODE");
    break;
    
  case CFG_MODE_KEYPAD:
    sprintf(posmode, "KEYPAD MODE");
    break;
    
  case CFG_MODE_POS:
    sprintf(posmode, "POS MODE");
    break;
    
  default:
    sprintf(posmode, "POS MODE");
    break;
  }

  switch(PoscvtCfg_g.baudrate)
  {
  case CFG_BAUD_9600:
    baudRate = 9600;
    break;
    
  case CFG_BAUD_19200:
    baudRate = 19200;
    break;
    
  case CFG_BAUD_38400:
    baudRate = 38400;
    break;
    
  case CFG_BAUD_57600:
    baudRate = 57600;
    break;
    
  case CFG_BAUD_115200:
    baudRate = 115200;
    break;

  default:
    baudRate = 9600;
    break;
  }
  sprintf(tempBuff, "MODE : %s\n", posmode);
  uart_putstring((uint8_t*)tempBuff);
  
  sprintf(tempBuff, "BAUDRATE : %d\n", baudRate);
  uart_putstring((uint8_t*)tempBuff);
  if(change){
    char prtType[30];
    switch(PoscvtCfg_g.print_type)
    {
    case CFG_PRINT_A:
        nrf_gpio_pin_set(RS232_IN1);
        nrf_gpio_pin_clear(POS_DIR);
        sprintf(prtType, "Type A"); //RJ45 2번 PRINT_TX , 3번 PRINT_RX
        break;
    
    case CFG_PRINT_B:
        nrf_gpio_pin_clear(RS232_IN1);
        nrf_gpio_pin_set(POS_DIR);
        sprintf(prtType, "Type B"); //RJ45 2번 PRINT_RX , 3번 PRINT_TX
        break;
    
    case CFG_PRINT_C:
        sprintf(prtType, "Type C");
        break;
    
    default:
        sprintf(prtType, "Type A");
        break;
    }
    sprintf(tempBuff, "Print type : %s\n", prtType);
    uart_putstring((uint8_t*)tempBuff);
  
    sprintf(tempBuff, "Volume : %d\n", ((16 - PoscvtCfg_g.volume) / 4) + 1);
    uart_putstring((uint8_t*)tempBuff);
    print_compiled_date(true);
    //sprintf(tempBuff, "F/W Ver: %s\n", __DATE__);
    //uart_putstring((uint8_t*)tempBuff);
  }
  else{
    print_compiled_date(false);
  }
  sprintf(tempBuff, "ABCDEFG\n");
  uart_putstring((uint8_t*)tempBuff);
#ifdef POS_PARSER_ENABLE
  sprintf(tempBuff, "CPK:%s\n",pos_parser_get_CPK());
  uart_putstring((uint8_t*)tempBuff);
  sprintf(tempBuff, "TPK:%s\n",pos_parser_get_TPK());
  uart_putstring((uint8_t*)tempBuff);
#endif   
  sprintf(tempBuff, "가나다라마바\n\n\n\n\n%c%c", 0x1B, 0x69);
  uart_putstring((uint8_t*)tempBuff);  
  if(PoscvtCfg_g.mode == CFG_MODE_KEYPAD && change == false){
      set_print_baud_rate(PoscvtCfg_g.keypad_baudrate); // Default
  }
}

/*
*@ brief: 메모리에 저장된 영수증 데이터를 없애는 과정
*/
void clear_receipt(void)
{
  uint8_t tempBuff[10];

  tempBuff[0] = (uint8_t)(0>>8);
  tempBuff[1] = (uint8_t)0;
  eep_write_array(0, tempBuff, 2);                //write length of data at 0
  while(IsSPIBusy()) 
  {
    //NRF_WDT->RR[0] = WDT_RR_RR_Reload;            //clear watchdog
  }

  bleQueue_g.data_idx_tail = 0;
  bleQueue_g.data_idx_hdr = 0;  
  
  bleQueue_g.CAT2Len = 0;
  memset(bleQueue_g.CAT2Buff, 0x00, 20);

  PoscvtCfg_g.IsReceiptdata = false;
  //SEGGER_RTT_printf(0, "\nSYSEVT_BLINK_LED OFF");
  clr_SysEvent(SYSEVT_BLINK_LED);               //to stop blink LED
}

/*
*@ brief: 영수증 데이터 출력
*/
void print_receipt(void)
{
  uint16_t addr = EEP_RECEIPT_ADDR;
  uint8_t tempBuff[10];
  uint16_t dataLen;
  bool endCheck = false;

  eep_read_array(0, tempBuff, 2, receipt_handler);
  while(IsSPIBusy()) 
  {
    //NRF_WDT->RR[0] = WDT_RR_RR_Reload;            //clear watchdog
  }     

  dataLen = (uint16_t)tempBuff[3];
  dataLen <<= 8;
  dataLen += (uint16_t)tempBuff[4];

  if(dataLen == 0) return;

  //SEGGER_RTT_printf(0, "\nloaded:%d", dataLen);
  while(dataLen != 0)
  {
    eep_read_array(addr, tempBuff, 1, receipt_handler);
    while(IsSPIBusy()) 
    {
      NRF_WDT->RR[0] = WDT_RR_RR_Reload;            //clear watchdog
    }     
    
    //SEGGER_RTT_printf(0, "%c", tempBuff[3]);
    uart_put(tempBuff[3]);

    ++addr;
    --dataLen;

    if(tempBuff[3] == 0x1B)
    {
      endCheck = true;
    }
    else if((endCheck == true) && (tempBuff[3] == 0x69))
    {
      //break;
      if(dataLen < 5) break;
    }
    else
    {
      endCheck = false;
    }
  }
}

/*
*@ brief: 영수증 데이터 출력
*/
unsigned char new_print_receipt(bool first)
{
static uint16_t receipt_addr = EEP_RECEIPT_ADDR;
static uint16_t receipt_dataLen = 0;   
static bool     receipt_endCheck = false;
  uint8_t tempBuff[10];

  if(first){
    receipt_endCheck = false;
    receipt_addr = EEP_RECEIPT_ADDR;
    eep_read_array(0, tempBuff, 2, receipt_handler);
    while(IsSPIBusy()) 
    {
       //NRF_WDT->RR[0] = WDT_RR_RR_Reload;            //clear watchdog
    }     
    receipt_dataLen = (uint16_t)tempBuff[3];
    receipt_dataLen <<= 8;
    receipt_dataLen += (uint16_t)tempBuff[4];
    if(receipt_dataLen == 0){ 
        return 0;
    }
  }
  else{
      if(receipt_dataLen == 0)
          return 1;
  }
  eep_read_array(receipt_addr, tempBuff, 1, receipt_handler);
  while(IsSPIBusy()) 
  {
    NRF_WDT->RR[0] = WDT_RR_RR_Reload;            //clear watchdog
  }        
  uart_put(tempBuff[3]);
  ++receipt_addr;
  --receipt_dataLen;
  if(tempBuff[3] == 0x1B)
  {
    receipt_endCheck = true;
  }
  else if((receipt_endCheck == true) && (tempBuff[3] == 0x69))
  {
    if(receipt_dataLen < 5)
        receipt_dataLen = 0;
  }
  else
  {
    receipt_endCheck = false;
  }
  if(receipt_dataLen == 0){
    receipt_addr = EEP_RECEIPT_ADDR;
    receipt_endCheck = false;
    return 3;
  }
  return 2;
}

/*
*@ brief: CAT2 모드에 대한 응답
*/
void sendCAT2Succeed(void)
{
  char tx_data[32];

  NRF_UART0->TASKS_STARTTX = 0;
  
  NRF_UART0->PSELTXD = CAT2_PIN_NUMBER;                  //to CAT2
  nrf_gpio_pin_set(CAT2_PIN_NUMBER);
  nrf_gpio_pin_set(TX_PIN_NUMBER);

  sprintf(tx_data, "RDY\r\n");
  uart_putstring((uint8_t*)tx_data);
  
  bleQueue_g.CAT2Len = 0;
  NRF_UART0->PSELTXD = TX_PIN_NUMBER;                   //to printer
  nrf_gpio_pin_set(CAT2_PIN_NUMBER);
  nrf_gpio_pin_set(TX_PIN_NUMBER);
    
  NRF_UART0->TASKS_STARTTX = 1;
}

/*
* @brief : $END응답 보내기
*/
void send_endQR(void)
{
  char tx_data[32];
  sprintf(tx_data, "#####");
  send_responseToBLE(tx_data);
}

/*
*@ brief: Base64 Encryption Susy 체크
*/
void clearEncBusy(void)
{
  if(ble_send_packet_count > 0){
      ble_send_packet_count--;
  }
  bleQueue_g.IsBusy = false;
}

/*
*@ brief: Base64용 버퍼 셋업
*/
int setBase64Buffer(uint8_t *inBuff, uint16_t len)
{
  bleQueue_g.base64Len = BLE_QUEUE_SIZE;

  return base64_encode(bleQueue_g.base64Buff, &bleQueue_g.base64Len, inBuff, len);
}

/*
*@brief : 메모리에서 옵션값을 불러오기
*/
void load_qr_code(void)
{
  uint32_t qrcode_length;
  uint16_t eep_addr = EEP_QRDATA_ADDR;
  uint32_t flashAddr = FLASH_QRCODE_ADDR;
  uint32_t i;
  uint32_t tempUINT32;
  uint8_t tempBuff[10];
  uint32_t remainder;
  uint32_t read_length;
  eep_write_status(WRITE_MODE_SEQ);                                                   //SRAM 메모리를 SEQ모드로 세팅
    
  qrcode_length = read_length = nd_nvmc_read(flashAddr);
  if(qrcode_length != 0xFFFFFFFFU)                  //It has QR code
  {
    //SEGGER_RTT_printf(0, "\nstored qr@flash:%d", qrcode_length);

    tempBuff[0] = (uint8_t)(qrcode_length>>8);
    tempBuff[1] = (uint8_t)qrcode_length;
    eep_addr = EEP_QRINFO_ADDR;
    eep_write_array(eep_addr, tempBuff, 2);              //write length of QR code
    while(IsSPIBusy()) 
    {
      //NRF_WDT->RR[0] = WDT_RR_RR_Reload;            //clear watchdog
    }     

    flashAddr = FLASH_QRCODE_ADDR + 4;                  //4 is header, length of QR code
    eep_addr = EEP_QRDATA_ADDR;
    
    remainder = qrcode_length % 4;
    if(remainder != 0){
        read_length += (4 - remainder);
    }
    for(i=0; i< read_length; ++i)               //0x100 is (0x400 / 4) = 1024 byte
    {
      tempUINT32 = nd_nvmc_read(flashAddr + i * 4);

      tempBuff[3] = (uint8_t)(tempUINT32>>24);
      tempBuff[2] = (uint8_t)(tempUINT32>>16);
      tempBuff[1] = (uint8_t)(tempUINT32>>8);
      tempBuff[0] = (uint8_t)tempUINT32;
      eep_write_array(eep_addr + i*4, tempBuff, 4);
      while(IsSPIBusy()) 
      {
        //NRF_WDT->RR[0] = WDT_RR_RR_Reload;            //clear watchdog
      }     
    }

    bleQueue_g.QRData.idx_tail = 0;
    bleQueue_g.QRData.idx_hdr = (uint16_t)qrcode_length;
  }
  else
  {
    
    bleQueue_g.QRData.idx_hdr = 0;
    bleQueue_g.QRData.idx_tail = 0;
  }
}

/*
* @brief : BLE로 수신한 명령어 처리
*/
void parser_data(uint8_t const* pBuff, uint16_t len, ble_nus_t* m_nus, uint16_t handle)
{
  char data[EEP_BUFF_SIZE + 1] = {0,};
  uint8_t tx_data[20];
  uint32_t i=0;
  uint8_t data_len;
  
  pNus_g = m_nus;
  handle_g = handle;

  memset(data, 0x00, sizeof(data));
  memset(tx_data, 0x00, sizeof(tx_data));
  if(len >= EEP_BUFF_SIZE) len = EEP_BUFF_SIZE;

  memcpy(data, pBuff, len);
  data[len] = 0x00;

  data_len = strlen(data);

  if(strcmp(data, "tag") == 0)               //send receipt data
  {
    SEGGER_RTT_printf(0, "\n%d [App -> MCU BLE Rx]%s",get_tickcount(), data);
    if(PoscvtCfg_g.ble_tag == false){
        PoscvtCfg_g.ble_tag = true;
    }
  }
  else if(strcmp(data, "wait") == 0)              //wait command
  {
    SEGGER_RTT_printf(0, "\n%d [App -> MCU BLE Rx]%s",get_tickcount(), data);
    if(PoscvtCfg_g.ble_wait == false){
        PoscvtCfg_g.ble_wait = true;
    }
    reverseDataCount_g = 0;
  }
  else if(strcmp(data, "PRF") == 0)              //Print fail
  {
    SEGGER_RTT_printf(0, "\n%d [App -> MCU BLE Rx]%s",get_tickcount(), data);
    if(PoscvtCfg_g.ble_prf == false){
        PoscvtCfg_g.ble_tag_connection = false;		//200216 KSD
        PoscvtCfg_g.ble_prf = true;
    }
  }
  else if(strncmp(data, "OPT:", 4) == 0)              //set options
  {
    if(PoscvtCfg_g.ble_opt == false){
      SEGGER_RTT_printf(0,"\r\n OPT:%c%c%c%c", data[4], data[5], data[6], data[7]);
      PoscvtCfg_g.mode = data[4] - '0' + CFG_MODE_POS;
      set_print_mode(data[5] - '0' + CFG_PRINT_A );
      PoscvtCfg_g.baudrate = data[6] - '0' + CFG_BAUD_9600; 
      set_print_baud_rate(data[6] - '0' + CFG_BAUD_9600 );
      setVolume(data[7] - '0');
			
      sprintf((char*)tx_data, "OPT:%c%c%c%c", data[4], data[5], data[6], data[7]);
      send_responseToBLEinEvent((char *)tx_data);
      PoscvtCfg_g.ble_opt = true;
    }
  }
  else if(strcmp(data, "cancel") == 0)              //cancel all, send
  {
    SEGGER_RTT_printf(0, "\n%d [App -> MCU BLE Rx]%s",get_tickcount(), data);
    if(PoscvtCfg_g.ble_cancel == false){
      PoscvtCfg_g.ble_cancel = true;
    }
  }
  else if(strncmp(data, "PRINT", 5) == 0)              //print receipt
  {
    if(PoscvtCfg_g.ble_print == false){
      PoscvtCfg_g.ble_print = true;
    }
  }      
  else if(strcmp(data, ",QR$") == 0)              //QR embedding
  {
    if(PoscvtCfg_g.ble_qr == false){
      PoscvtCfg_g.ble_qr = true;
    }
  }
  else if(strncmp(data + data_len - 3, "QD$", 3) == 0)              //QR data command
  {
    if(PoscvtCfg_g.ble_qd == false){
      if(data[data_len - 4] == '1')   
        bleQueue_g.QRData.idx_hdr = 0;
      setQRData((uint8_t*)data, len-5);
      PoscvtCfg_g.ble_qd = true;
    }
  }
  else if(strncmp(data, "CPK#", 4) == 0)              //set CPK
  {
    if(PoscvtCfg_g.ble_cpk == false){
      PoscvtCfg_g.ble_cpk = true;
      pos_parser_set_CPK(true,(unsigned char *)&data[4],strlen((char *)&data[4]));
      SEGGER_RTT_printf(0,"\r\n[APP->MCU] %s",data);
    }
  }
  else if(strncmp(data, "TPK#", 4) == 0)              //set TPK
  {
    if(PoscvtCfg_g.ble_tpk == false){
      PoscvtCfg_g.ble_tpk = true;
      pos_parser_set_TPK((unsigned char *)&data[4],strlen((char *)&data[4]));
    }
  }
  else if(strncmp(data + data_len - 3, "RD$", 3) == 0)              //Reverse data command
  {
    if(PoscvtCfg_g.ble_rd == false){
      PoscvtCfg_g.ble_rd = true;
      targetDataCount_g = data[data_len - 5] - '0';
      targetDataCount_g *= 10;
      targetDataCount_g |= data[data_len - 4] - '0';
      setReverseData((uint8_t*)data, data_len-6, false);
      ++reverseDataCount_g;
      if(reverseDataCount_g >= targetDataCount_g){
        //set_SysEvent(SYSEVT_WRITE_REVDATA);
        PoscvtCfg_g.ble_wr_rd = true;
      }
    }
  }
  else if(strncmp(data + data_len - 4, "PRS1", 4) == 0){
    uint32_t num;
    if(PoscvtCfg_g.ble_prs1 == false){
      bleQueue_g.sentSuccess = true;
      for(i=0, num=0; i< (data_len - 1); ++i){          //ex; $1500,PRS1
        if(data[i+1] == ',') 
          break;
        num *= 10;
        num += data[i + 1] - '0';
      }
      if(num > 9999999) num = 9999999;
         setSoundTotal(num);
      PoscvtCfg_g.ble_tag_connection = false;		//200216 KSD
      PoscvtCfg_g.ble_prs1 = true;
    }
  }
  else if(strncmp(data + data_len - 4, "PRS0", 4) == 0){
    uint32_t num;
    if(PoscvtCfg_g.ble_prs0 == false){
      bleQueue_g.sentSuccess = true;
      for(i=0, num = 0; i< (data_len - 1); ++i){          //ex; $1500.PRS0
        if(data[i+1] == ',') 
          break;
        num *= 10;
        num += data[i + 1] - '0';
      }
      if(num > 9999999) num = 9999999;
          setSoundTotal(num);
      PoscvtCfg_g.ble_tag_connection = false;		//200216 KSD
      PoscvtCfg_g.ble_prs0 = true;
    }
  }
#ifdef ENABLE_SOA_DFU
  else if(strncmp(data, "PC_FW_UPDATE",12) == 0){
      if(PoscvtCfg_g.ble_fum == false){
        PoscvtCfg_g.ble_fum = true;
      }
  }
#endif
}

/*
* @brief : UART 데이터 처리
*/
void pos_cnt_uart_parser(void)
{
  static uint32_t timeout = 0;
  static uint16_t eep_addr = EEP_RECEIPT_ADDR;
  uint16_t write_addr = 0;
  uint8_t rxd_data = 0;
  uint8_t tempBuff[10];
//BLE Auto cut off 기능추가 2020.02.07 ksd
  uint32_t num;
  uint32_t i=0;

  if(uart_get_RXD(&rxd_data) == true)
  {
    timeout = get_tickcount();
    if(receiptFSM(RECEIPT_FSM_WRITE) == true)
    {
      eep_addr = EEP_RECEIPT_ADDR;
      bleQueue_g.data_idx_tail = 0;
      bleQueue_g.data_idx_hdr = 0;
      PoscvtCfg_g.IsReceiptdata = false;
      bleQueue_g.CAT2Len = 0;
      pos_parser_reset();
      set_SysEvent(SYSEVT_ADV_STOP);
    }

    if(getReceiptFSM() == RECEIPT_FSM_WRITE)
    {
      pos_parser_push(rxd_data);
      if(PoscvtCfg_g.mode == CFG_MODE_KEYPAD) // cat2 mode
      {
        bleQueue_g.tempCAT2Buff[bleQueue_g.CAT2Len++] = rxd_data;
        bleQueue_g.CAT2Len %= 20;

        eep_addr = EEP_RECEIPT_ADDR;
        bleQueue_g.data_idx_tail = 0;
        bleQueue_g.data_idx_hdr = 0;
      }
      else if(PoscvtCfg_g.mode == CFG_MODE_CAT) // cat mode
      {
        
      }
      else // pos mode
      {
        tempBuff[0] = rxd_data;
        eep_write_array(eep_addr, tempBuff, 1);
        while(IsSPIBusy()) 
        {
          //NRF_WDT->RR[0] = WDT_RR_RR_Reload;            //clear watchdog
        }
        eep_addr++;

        if(eep_addr > EEP_ADDR_LIMIT) 
        {
          eep_addr = EEP_RECEIPT_ADDR;
          bleQueue_g.data_idx_tail = 0;
          bleQueue_g.data_idx_hdr = 0;
        }
      }
    }
    else
    {
      //Do nothing
    }
  }
  else if(getReceiptFSM() == RECEIPT_FSM_CLEAN)
  {
    if(process_over_time(timeout,10))                //over 100mSec
    {
      timeout = get_tickcount();

      write_addr = 0;

      tempBuff[0] = (uint8_t)(write_addr>>8);
      tempBuff[1] = (uint8_t)write_addr;
      
      eep_write_array(0, tempBuff, 2);                //write length of data at 0
      while(IsSPIBusy()) 
      {
        //NRF_WDT->RR[0] = WDT_RR_RR_Reload;            //clear watchdog
      }

      bleQueue_g.data_idx_tail = 0;
      bleQueue_g.data_idx_hdr = write_addr;
      
      receiptFSM(RECEIPT_FSM_READY);
      pos_parser_reset();  
    }
  }
  else if(getReceiptFSM() == RECEIPT_FSM_AVAILABLE)
  {
    if(process_over_time(timeout,10))                //over 100mSec
    {
      timeout = get_tickcount();

      receiptFSM(RECEIPT_FSM_READY);
    }
  }
  else if(process_over_time(timeout,30))             //over 500mSec -> 300mSec 0209 KSD.MODIFY
  {
    timeout = get_tickcount();
    if(getReceiptFSM() == RECEIPT_FSM_WRITE)
    {
      set_SysEvent(SYSEVT_ADV_START);
      write_addr = eep_addr - EEP_RECEIPT_ADDR;               //10 is start address

      if(PoscvtCfg_g.mode == CFG_MODE_POS)          //POS mode, check length of receipt
      {
        if(write_addr < 100)
        {
          write_addr = 0;
        }
      }
      else if(PoscvtCfg_g.mode == CFG_MODE_KEYPAD)
      {
        bleQueue_g.tempCAT2Buff[bleQueue_g.CAT2Len] = 0x00;
        if(bleQueue_g.CAT2Len < 5) bleQueue_g.CAT2Len = 5;
        if(strncmp((char*)(bleQueue_g.tempCAT2Buff + bleQueue_g.CAT2Len - 4), (char*)",SUM", 4) == 0)                              //CAT2 data
        {
          if(bleQueue_g.tempCAT2Buff[0] == ' ')                                                                    //check space
          {
            PoscvtCfg_g.IsReceiptdata = true;
            receiptFSM(RECEIPT_FSM_AVAILABLE);
            setCAT2Data(bleQueue_g.tempCAT2Buff + 1, bleQueue_g.CAT2Len - 1);
						
            //BLE Auto cut off 기능추가 2020.02.07 ksd
            //[BEFORE]
            //RECEIPT_SOUND;
						
            //[AFTER]
            for(i=0, num = 0; i< bleQueue_g.CAT2Len; ++i){          //ex; $1500.PRS0
              if(bleQueue_g.tempCAT2Buff[i+1] == ',') 
                break;
              num *= 10;
              num += bleQueue_g.tempCAT2Buff[i + 1] - '0';
            }
            if(num > 9999999) num = 9999999;
            setSoundTotal(num);
		
            last_sound = false;
            set_SysEvent(SYSEVT_PLAY_SOUND);
						
            set_SysEvent(SYSEVT_BLINK_LED);     
          }
          else
          {
            write_addr = 0;
            bleQueue_g.CAT2Len = 0;
          }
        }
        else
        {
          write_addr = 0;
          bleQueue_g.CAT2Len = 0;
        }
      }
      else //CAT mode
      {
        write_addr = 0;
        PoscvtCfg_g.IsReceiptdata = false;
      }

      tempBuff[0] = (uint8_t)(write_addr>>8);
      tempBuff[1] = (uint8_t)write_addr;
      
      if(write_addr != 0)
      {
        PoscvtCfg_g.IsReceiptdata = true;
        receiptFSM(RECEIPT_FSM_AVAILABLE);
        
        //RECEIPT_SOUND;
        if(PoscvtCfg_g.mode == CFG_MODE_POS)
        {
          pos_parser_push(0x0A); // end
          if(pos_data_is_card())
          {
            set_SysEvent(SYSEVT_FORCE_PRINT);
          }
          else
          {
            RECEIPT_SOUND;
            set_SysEvent(SYSEVT_BLINK_LED);  
          }
        } 
      }
      else{
          receiptFSM(RECEIPT_FSM_AVAILABLE);
      }
      if(PoscvtCfg_g.mode == CFG_MODE_POS) 
      {
        eep_write_array(0, tempBuff, 2);                //write length of data at 0
        while(IsSPIBusy()) 
        {
          //NRF_WDT->RR[0] = WDT_RR_RR_Reload;            //clear watchdog
        }
      }
      bleQueue_g.data_idx_tail = 0;
      bleQueue_g.data_idx_hdr = write_addr;
    }
  }
}

void send_modeSum(bool parsing)
{
  char tx_data[32];
  if(PoscvtCfg_g.mode == CFG_MODE_CAT){
    sprintf(tx_data, "PM2#####0,$SUM");
  }
  else if(PoscvtCfg_g.mode == CFG_MODE_KEYPAD){
    if(bleQueue_g.CAT2Len != 0)
    {
      sprintf(tx_data, "PM3#####%s", bleQueue_g.CAT2Buff);
    }
    else
    {
      sprintf(tx_data, "PM3#####N,$SUM");
    }
  }
  else{
    if(parsing){
      sprintf(tx_data, "PM1#####%d,$SUM",pos_parser_get_payment());
    }
    else{
      sprintf(tx_data, "PM1#####N,$SUM");

      //BLE Auto cut off 기능추가 2020.02.07 ksd
      no_receipt_flag = true;
   }
  }
  send_responseToBLE(tx_data);
}

void send_PRS(void)
{
  char tx_data[8];
  sprintf(tx_data, "$PRS");
  send_responseToBLE(tx_data);
}

void send_DIS(void)
{
  char tx_data[8];
  sprintf(tx_data, "$DIS");
  send_responseToBLE(tx_data);
}

void send_PRF(void)
{
  char tx_data[8];
  sprintf(tx_data, "$PRF");
  send_responseToBLE(tx_data);
}

void send_FUM(void)
{
  char tx_data[8];
  sprintf(tx_data, "$FUM");
  send_responseToBLE(tx_data);
}

void send_responseToBLE(char *pStr)
{
    uint32_t wait_time = get_tickcount();
    gen_qode_packet(pNus_g, (uint8_t*)pStr, strlen(pStr), handle_g);
    ble_send_packet_count++;
    while(ble_send_packet_count > 0){
        if(process_over_time(wait_time,20)){ // 200ms
            ble_send_packet_count = 0;
            break;
        }
    }
}

void send_responseToBLEinEvent(char *pStr)
{
    gen_qode_packet(pNus_g, (uint8_t*)pStr, strlen(pStr), handle_g);
    ble_send_packet_count++;
}

void process_wait_complete_packet(void)
{
    uint32_t wait_time = get_tickcount();
    while(ble_send_packet_count > 0){
        if(process_over_time(wait_time,20)){ // 200ms
            ble_send_packet_count = 0;
            break;
        }
    }
}


