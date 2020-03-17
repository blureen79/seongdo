#include "custom_board.h"
#include "func.h"
#include "nrf_wdt.h"
#include "nrfx_wdt.h"
#include "nrf_gpio.h"
#include "nrf_delay.h"
#include "nrf_fstorage.h"
#include "nrf_fstorage_sd.h"
#include "nrf_drv_gpiote.h"
#include "ndevice_pwm.h"
#include "ndevice_timer.h"
#include "process_data.h"
#include "ndevice_twim.h"
#include "ndevice_nvmc.h"
#include "ndevice_uart.h"
#include "ndevice_util.h"
#include "ndevice_eeprom.h"
#include "ndevice_sadc.h"
#include "SEGGER_RTT.h"
#include "shoponair.h"

typedef enum
{
    NRF_FSTORAGE_STATE_IDLE,            //!< No operations requested to the SoftDevice.
    NRF_FSTORAGE_STATE_OP_PENDING,      //!< A non-fstorage operation is pending.
    NRF_FSTORAGE_STATE_OP_EXECUTING,    //!< An fstorage operation is executing.
} nrf_fstorage_sd_state_t;

typedef struct
{
    nrf_atomic_flag_t       initialized;    //!< fstorage is initalized.
    nrf_atomic_flag_t       queue_running;  //!< The queue is running.
                                            /** Prevent API calls from entering queue_process(). */
    nrf_fstorage_sd_state_t state;          //!< Internal fstorage state.
    uint32_t                retries;        //!< Number of times an operation has been retried on timeout.
    bool                    sd_enabled;     //!< The SoftDevice is enabled.
    bool                    paused;         //!< A SoftDevice state change is impending.
                                            /** Do not load a new operation when the last one completes. */
} nrf_fstorage_sd_work_t;

typedef struct
{
  uint32_t address;
  uint32_t buff[32];
} stLastFlashInfo;

typedef struct
{
  uint32_t led_tick;
  uint32_t button_tick;
  uint32_t reamin_conn_tick;
  uint32_t qr_write_tick;
  uint32_t ble_wait_tick;
  uint32_t ble_disconect_wait;
	uint32_t segment_tick;
} stTickcounter;

typedef struct
{
  uint8_t buff[2048];
  uint16_t len;
} stReverseData;

bool bTimer_handled_g = false;

static uint16_t sysevent_g = 0;
static uint32_t sys_tickcount_g = 0;
static bool running_advertising = false;
uint32_t wait_cnt = 0;
uint32_t segment_position = 0;
uint32_t segment_wait_cnt = 0;

unsigned char segnum[10] = {
	63,
	6,
	91,
	79,
	102,
	109,
	124,
	39,
	127,
	103
};

//From main.c
extern bool IsConnected(void);              //It from main.c
extern void nd_disconnect(void);            //It from main.c
extern void advertising_stop(void);         //It from main.c

//BLE Auto cut off 기능추가 2020.02.07 ksd
extern int EVTCH_RSSI;								//It from main.c
extern uint8_t macaddr[6];						//It from main.c
extern bool last_sound;								//It from process_data.c
extern bool prf_cancel_flag;					//It from process_data.c
extern bool no_receipt_flag;					//It from process_data.c
extern bool print_pass_flag;					//It from process_data.c
//End of extern functions

static uint16_t receiptState_g = RECEIPT_FSM_READY;
stTickcounter Tickcounter_g;
stPOSCVTcfg PoscvtCfg_g;
stReverseData reverseData_g;
uint8_t sndToTal_g[20];                                 //sound of total amount

//TWIS I2C CODE
#define TX_BUFFER_SIZE 255
#define RX_BUFFER_SIZE 255

void twisTX(const char *string);  // Master로 데이터 보냄
bool twisRX(void);            // Master에서 데이터 읽음
bool twisCheck(void);         // Master와 연결되었는지 확인

typedef struct {
  uint8_t tx_buffer[TX_BUFFER_SIZE];
  uint8_t rx_buffer[RX_BUFFER_SIZE];
}BufferArray_Type;

BufferArray_Type Buffer;

char RECVdata[RX_BUFFER_SIZE] = {0,};
char Senddata[RX_BUFFER_SIZE] = {0,};

void twisTX(const char *string){
  // Master에서 데이터를 요청할 때 사용하는 함수
  // Read 명령을 수신하였는지 확인
  if(NRF_TWIS0->EVENTS_READ){
    // 이벤트 플래그 초기화
    NRF_TWIS0->EVENTS_READ = 0;

    // 보낼 데이터의 크기 설정
    NRF_TWIS0->TXD.MAXCNT = sizeof(Buffer.tx_buffer);

    // 데이터 버퍼 설정
    memcpy(Buffer.tx_buffer, string, strlen(string));
    
    NRF_TWIS0->TXD.PTR = (uint32_t)&Buffer.tx_buffer[0];

    // PREPARETX 플래그 설정
    NRF_TWIS0->TASKS_PREPARETX = 1;

    if(NRF_TWIS0->EVENTS_TXSTARTED){
                   //SEGGER_RTT_printf(0, "\nTX Started!");
    }
  }
}

int BuffInCnt = 0;
int BuffOutCnt = 0;

bool twisRX(void){
  // Master에서 보낸 데이터를 읽는 함수
  // Write 명령을 수신하였는지 확인
  if(NRF_TWIS0->EVENTS_WRITE){

    // 이벤트 플래그 초기화
    NRF_TWIS0->EVENTS_WRITE = 0;
    
    // 받을 데이터의 크기 설정
    NRF_TWIS0->RXD.MAXCNT = sizeof(Buffer.rx_buffer);

    // 데이터 버퍼 설정
    NRF_TWIS0->RXD.PTR = (uint32_t)&Buffer.rx_buffer[0];
    
    //memcpy(RECVdata, &Buffer.rx_buffer[0], NRF_TWIS0->RXD.AMOUNT);
    for(int idx=0;idx<NRF_TWIS0->RXD.AMOUNT;++idx){
        RECVdata[BuffInCnt] = Buffer.rx_buffer[idx];
        BuffInCnt = (BuffInCnt+1)%RX_BUFFER_SIZE;
    }
    
    // PREPARERX 플래그 설정
    NRF_TWIS0->TASKS_PREPARERX = 1;

    return 1;
  }else{
    return 0;
  }
}

bool twisCheck(void){
  // 마스터와 연결 되었는지 확인하는 함수
  if(NRF_TWIS0->MATCH){
    //SEGGER_RTT_printf(0, "\nConnected with Master");
    return 1;
  }
  return 0;
}

//fstorage lib
static void fstorage_handler(nrf_fstorage_evt_t * p_evt);

nrf_fstorage_api_t * p_fs_api_g;

NRF_FSTORAGE_DEF(nrf_fstorage_t fstorage) =
{
    .evt_handler    = fstorage_handler,
    .start_addr     = FLASH_OPTION_ADDR,
    .end_addr       = FLASH_QRCODE_ADDR + FLASH_QRCODE_LIMIT,               //size is limited to 84kbytes
};
//end of fsorage

//#ifdef NO_SEGGER
//int SEGGER_RTT_printf(unsigned BufferIndex, const char * sFormat, ...) {

//  return 0;
//}
//#endif

/*
*@brief: byte array to uint32
*/
void bytearrToUINT32(uint8_t *pBuff, uint32_t *pOut)
{
  uint8_t param;
  int i;
  uint32_t tempUIN32 = 0;
  
  for(i=0; i<4; ++i)
  {
    if(pBuff[i*2] <= '9') param = pBuff[i*2] - '0';
    else if((pBuff[i*2] <= 'z') && (pBuff[i*2] >= 'a')) param = pBuff[i*2] - 'a' + 10;
    else 
    {
      param = pBuff[i*2] - 'A' + 10;
    }    
    param <<= 4;
    
    if(pBuff[i*2 + 1] <= '9') param += pBuff[i*2 + 1] - '0';
    else if((pBuff[i*2 + 1] <= 'z') && (pBuff[i*2 + 1] >= 'a')) param += pBuff[i*2 + 1] - 'a' + 10;
    else 
    {
      param += pBuff[i*2 + 1] - 'A' + 10;
    }        

    tempUIN32 <<= 8;
    tempUIN32 += (uint32_t)param;
  }

  *pOut = tempUIN32;
}

static bool func_over_time(uint32_t ref_times, uint32_t diff_times)
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
*@brief : QR처리 timeout 리셋
*/
void resetQRWriteTimer(void)
{
  Tickcounter_g.qr_write_tick = get_tickcount();
}

/*
*@brief : BLE연결에 대한 timeout 리셋
*/
void resetRemainConnect(void)
{
  PoscvtCfg_g.IsTagReceived = true;
  Tickcounter_g.reamin_conn_tick = get_tickcount();
}

uint16_t getReceiptFSM(void)
{
  return receiptState_g;
}

/*
*@ brief : 영수증데이터 처리를 위한 FSM
*/
bool receiptFSM(uint16_t state)
{
  bool ret = false;
  
  switch(receiptState_g)
  {
  case RECEIPT_FSM_READY:                       //영수증은 write 또는 read 될 수 있음
    if(state == RECEIPT_FSM_WRITE)
    {
      receiptState_g = RECEIPT_FSM_WRITE;
      
      ret = true;
    }
    else if(state == RECEIPT_FSM_READ)
    {
      receiptState_g = RECEIPT_FSM_READ;
      
      ret = true;
    }
    break;
    
  case RECEIPT_FSM_READ:                        //영수증은 ready가 되기전에 clean을 먼저 세팅 될 수 있음
    if(state == RECEIPT_FSM_READY)
    {
      receiptState_g = RECEIPT_FSM_CLEAN;
      
      ret = true;
    }
    break;

  case RECEIPT_FSM_WRITE:                       //영수증은 ready 될 수 있음
    if(state == RECEIPT_FSM_AVAILABLE)
    {
      receiptState_g = RECEIPT_FSM_AVAILABLE;
      
      ret = true;
    }
    break;

  case RECEIPT_FSM_AVAILABLE:                       //영수증은 ready 될 수 있음
    if(state == RECEIPT_FSM_READY)
    {
      receiptState_g = RECEIPT_FSM_READY;
      
      ret = true;
    }
    break;

  case RECEIPT_FSM_CLEAN:                       //영수증은 ready 될 수 있음
    if(state == RECEIPT_FSM_READY)
    {
      receiptState_g = RECEIPT_FSM_READY;
      
      ret = true;
    }    
    break;

  default:
    receiptState_g = RECEIPT_FSM_READY;
    break;      
  }
  
  return ret;
}

/*
*@brief : BLE연결에 대한 timeout start
*/
void setTimeoutStart(void)
{
  Tickcounter_g.reamin_conn_tick = get_tickcount();

  bleQueue_g.data_idx_tail = 0;
  bleQueue_g.state = QUEUE_STANDBY;

  clearEncBusy();
  set_SysEvent(SYSEVT_TIMEOUTSTART);
}

/*
*@brief : timeout 이벤트처리 클리어
*/
void clrTimeout(void)
{
  clr_SysEvent(SYSEVT_TIMEOUTSTART);
}

/*
*@brief : TAG 명령어에 대한 상태 클리어
*/
void clrTagReceived(void)
{
  PoscvtCfg_g.IsTagReceived = false;
  SEGGER_RTT_printf(0,"\n%d BLE_GAP_EVT_DISCONNECTED" , get_tickcount());
}

/*
*@brief : 비정상 접속 에러 출력음성과 프린터 조건
*/
void Certified_error_call(void)
{
	char tempBuff[30];
	
	CERTIFIED_ERROR_SOUND;  //ERROR_SOUND;
	
	sprintf(tempBuff, "MAC ADDRESS: %02X:%02X:%02X:%02X:%02X:%02X",macaddr[5],macaddr[4],macaddr[3],
	                                                               macaddr[2],macaddr[1],macaddr[0]);
	uart_putstring((uint8_t*)tempBuff);
	
	sprintf(tempBuff, "\nRSSI: %d",EVTCH_RSSI);
	uart_putstring((uint8_t*)tempBuff);
	
	sprintf(tempBuff, "\n비정상 접속 입니다. \n\n\n\n\n%c%c", 0x1B, 0x69);
	uart_putstring((uint8_t*)tempBuff);
}

/*
*@brief : BLE 타임아웃 상태 확인 및 처리
*/
void checkRemainConnect(void)
{
  if (PoscvtCfg_g.rssi_limit == true) // RSSI LIMIT 초과 조건
  {
    PoscvtCfg_g.rssi_limit = false; 
    Tickcounter_g.reamin_conn_tick = get_tickcount();
    //Certified_error_call();		//200216 KSD
    SEGGER_RTT_printf(0, "\n%d Force Disconnect Sound(X)",get_tickcount());
    nd_disconnect();
    clr_SysEvent(~(SYSEVT_BLINK_LED));
  }
  else
  {
    if (PoscvtCfg_g.ble_tag_connection == true) //"tag" cmd receive
    {
      //PoscvtCfg_g.IsTagReceived = true; BLE CONNECT STATE , CMD RECEIVED
      if(func_over_time(Tickcounter_g.reamin_conn_tick, 300) == true)
      {
        Tickcounter_g.reamin_conn_tick = get_tickcount();
      
        FORCE_DISCONN_SOUND;  //"연결을 중단합니다"
        SEGGER_RTT_printf(0,"\n%d FORCE_DISCONN_SOUND 2" , get_tickcount());
        nd_disconnect();
        clr_SysEvent(~(SYSEVT_BLINK_LED));
      }
      else if(wait_cnt > 60) //0213 KSD , 60 x wait cmd 2sec = 120 sec
      {
        wait_cnt = 0;
        FORCE_DISCONN_SOUND;	//"연결을 중단합니다"
        SEGGER_RTT_printf(0,"\n%d FORCE_DISCONN_SOUND : 1 min over after tag" , get_tickcount());
        nd_disconnect();
        clr_SysEvent(~(SYSEVT_BLINK_LED));
      }
    }
    else // 비정상 접속 연결한 조건
    {
      //PoscvtCfg_g.IsTagReceived = false; BLE DISCONNECT STATE
      if(PoscvtCfg_g.IsTagReceived == false && func_over_time(Tickcounter_g.reamin_conn_tick, 300) == true)
      {
        SEGGER_RTT_printf(0,"\n%d BLE DISCONNECT STATE 3 SEC OVERTIME" , get_tickcount());
        nd_disconnect();
        clr_SysEvent(~(SYSEVT_BLINK_LED));
      }
    }
  }
}

/*
*@brief : QR데이터 저장
*/
void setQRData(uint8_t *pInBuff, uint16_t len)
{
  if((bleQueue_g.QRData.idx_hdr + len) >= EEP_QRDATA_LIMIT) 
  {
    return;
  }
  //SEGGER_RTT_printf(0, "\n#Debug - %d",__LINE__);
  //SEGGER_RTT_printf(0, "ADDRESS: %08X , Length: %d",bleQueue_g.QRData.data + bleQueue_g.QRData.idx_hdr , len);
  memcpy(bleQueue_g.QRData.data + bleQueue_g.QRData.idx_hdr, pInBuff, len);
  bleQueue_g.QRData.idx_hdr += len;  
  
  if(bleQueue_g.QRData.idx_hdr >= EEP_QRDATA_LIMIT) bleQueue_g.QRData.idx_hdr = EEP_QRDATA_LIMIT - 1;  
}

/*
*@brief : 리버스영수증 저장
*/
void setReverseData(uint8_t *pInBuff, uint16_t len, bool reset)
{
  if(reset == true) 
  {
    reverseData_g.len = 0;
    
    return;
  }
  memcpy(reverseData_g.buff + reverseData_g.len, pInBuff, len);
  reverseData_g.len += len;

  if(reverseData_g.len >= 2048) reverseData_g.len = 2048 - 1;
}

/*
*@brief : CAT2 데이터 저장
*/
void setCAT2Data(uint8_t *pInBuff, uint16_t len)
{
  unsigned char sum[32];
  unsigned char idx,pay_idx,str_len;
  memset(sum, 0, 32);
  pay_idx = 0;
  str_len = strlen((char *)pInBuff);
  if(str_len > 15)
      str_len = 15;
  for(idx = 0; idx < str_len; idx++){
    if(pInBuff[idx] >= '0' && pInBuff[idx] <= '9'){
        sum[pay_idx++] = pInBuff[idx];
    }
    else if(pInBuff[idx] == ',' && pay_idx != 0){
       break;
    }
  }
  if(pay_idx> 0){
     if(pay_idx > 15) 
         pay_idx = 15;
     sprintf((char *)bleQueue_g.CAT2Buff,"%s,$SUM",sum); //EX>" 500,SUM" 입력시 " 500,$SUM" 치환
  }
  bleQueue_g.sentSuccess = true;
}

void dummy_handler(void)
{

}
void HC595write(uint8_t uPosition,uint8_t udata)
{
	nrf_gpio_pin_clear(LATCH_CLOCK);
	
	uint8_t i = 7;
	
	while(i<=7)
  {
		if ((udata >> i) & 0x01)
		{
			nrf_gpio_pin_set(SHIFT_DATA);
		}
		else
		{
			nrf_gpio_pin_clear(SHIFT_DATA);
		}
		nrf_gpio_pin_set(SHIFT_CLOCK);
		nrf_gpio_pin_clear(SHIFT_CLOCK);
		i--;
	}
  
  i = 7;
	while(i<=7)
	{
		if((uPosition >> i) & 0x01)
		{
			nrf_gpio_pin_set(SHIFT_DATA);
		}
		else
		{
			nrf_gpio_pin_clear(SHIFT_DATA);			
		}
		nrf_gpio_pin_set(SHIFT_CLOCK);
		nrf_gpio_pin_clear(SHIFT_CLOCK);
		i--;
	} 
	nrf_gpio_pin_set(LATCH_CLOCK);
}

/*
*@brief : 리버스영수증 메모리에 저장
*/
void writeReverseToSRAM(void)
{
  uint16_t i;
  uint16_t eep_addr, write_addr, rev_addr;
  uint32_t len;
  uint16_t dlen = 2048;
  uint8_t tempBuff[64 + 1];
  
  eep_addr = EEP_RECEIPT_ADDR;
  rev_addr = EEP_REVDATA_ADDR;

  //SEGGER_RTT_printf(0, "reverseData_g.len:%d", reverseData_g.len);

  while(IsSPIBusy())
  {
    //NRF_WDT->RR[0] = WDT_RR_RR_Reload;            //clear watchdog
  }

  for(i = 0; i<reverseData_g.len; )
  {
    if((reverseData_g.len - i) > 64) 
    {
      eep_read_array(rev_addr + i, tempBuff, 64, dummy_handler);
      while(IsSPIBusy()) 
      {
        //NRF_WDT->RR[0] = WDT_RR_RR_Reload;            //clear watchdog
      }     

      base64_decode(bleQueue_g.base64Buff, &dlen, reverseData_g.buff + i, 64);
      
      eep_write_array(eep_addr + ((i * 6)>>3), bleQueue_g.base64Buff, dlen);
      i += 64;
      
      //for(int dbg_cnt=0; dbg_cnt<((i * 6)>>3); ++dbg_cnt) SEGGER_RTT_printf(0, "%c", bleQueue_g.base64Buff[dbg_cnt]);
    }
    else 
    {
      len = reverseData_g.len - i;
      
      eep_read_array(rev_addr + i, tempBuff, len, dummy_handler);
      while(IsSPIBusy()) 
      {
        //NRF_WDT->RR[0] = WDT_RR_RR_Reload;            //clear watchdog
      }     

      base64_decode(bleQueue_g.base64Buff, &dlen, reverseData_g.buff + i, len);
      eep_write_array(eep_addr + ((i * 6)>>3), bleQueue_g.base64Buff, dlen);
      i = reverseData_g.len;
    }

    while(IsSPIBusy())
    {
      //NRF_WDT->RR[0] = WDT_RR_RR_Reload;            //clear watchdog
    }

    if(eep_addr > EEP_ADDR_LIMIT) eep_addr = EEP_ADDR_LIMIT;    
  }

  //Write receipt header
  write_addr = eep_addr + ((reverseData_g.len * 6)>>3) - EEP_RECEIPT_ADDR;               //10 is start address
  tempBuff[0] = (uint8_t)(write_addr>>8);
  tempBuff[1] = (uint8_t)write_addr;
  eep_write_array(0, tempBuff, 2);                //write length of data at 0
  while(IsSPIBusy()) 
  {
    //NRF_WDT->RR[0] = WDT_RR_RR_Reload;            //clear watchdog
  }

  bleQueue_g.data_idx_tail = 0;
  bleQueue_g.data_idx_hdr = write_addr;

  PoscvtCfg_g.IsReceiptdata = true;
  //end of receipt header
}

void ctrlLEDState(void)
{
  static uint8_t secCount = 0;
  
  if(IsConnected() == true)
  {
    nrf_gpio_pin_clear(DEF_GPIO_RED1_PWM);              //LED always ON

    return;
  }

  if(bleQueue_g.data_idx_tail != bleQueue_g.data_idx_hdr) 
  {
    secCount = 0;
    nrf_gpio_pin_toggle(DEF_GPIO_RED1_PWM);
  }
  else if(PoscvtCfg_g.mode == CFG_MODE_CAT)
  {
    secCount = 0;
    nrf_gpio_pin_toggle(DEF_GPIO_RED1_PWM);
  }
  else if((PoscvtCfg_g.mode == CFG_MODE_KEYPAD) && (bleQueue_g.CAT2Len != 0))
  {
    secCount = 0;
    nrf_gpio_pin_toggle(DEF_GPIO_RED1_PWM);
  }
  else                  //no receipt
  {
    ++secCount;

    if(secCount == 10)
    {
      nrf_gpio_pin_clear(DEF_GPIO_RED1_PWM);
    }
    else if(secCount == 11)
    {
      nrf_gpio_pin_set(DEF_GPIO_RED1_PWM);
    }
    
    secCount %= 20;                 //reset every 10Seconds
  }
}

#ifdef ENABLE_SOA_DFU
#include "nrf_power.h"
#endif
/*
*@brief : 스테이트머신 처리
*/
extern void advertising_start(void);
static uint32_t elaspe_times;
void stick_fsm(bool reset)
{
  uint8_t tx_data[20];
  {
    bool isReceived = twisRX();
    int idx = 0;
    while(BuffInCnt!=BuffOutCnt)
    {
        Senddata[idx++] = RECVdata[BuffOutCnt];
        BuffOutCnt = (BuffOutCnt+1)%RX_BUFFER_SIZE;
    }
    Senddata[idx+0] = 0x00;
    Senddata[idx+1] = 0x00;
      
    if (Senddata[0] != 0x00 && Senddata[1] != 0x00)
    {
        if (Senddata[0] != 0x24 && Senddata[1] != 0x51)
        {
            uart_putstring((uint8_t*)Senddata);
        }
        else{         
            if(idx>7){
                uart_putstring((uint8_t*)&Senddata[7]);
            }
        }
    }
  }
  
  if(PoscvtCfg_g.ble_tag){
    receiptFSM(RECEIPT_FSM_READ);
    bleQueue_g.sentSuccess = false;
    resetRemainConnect();
    setReverseData(0, 0, true);
    set_SysEvent(SYSEVT_SEND_BLEDATA);
    PoscvtCfg_g.ble_tag_connection = true;	//0209 KSD.MODIFY
		
    PoscvtCfg_g.ble_tag = false;
  }
  else if(PoscvtCfg_g.ble_wait){
    wait_cnt++; //0212 KSD
    resetRemainConnect();
    PoscvtCfg_g.ble_wait = false;
  }
  else if(PoscvtCfg_g.ble_prf){
    //To avoid clean receipt
    receiptFSM(RECEIPT_FSM_READY);
    receiptFSM(RECEIPT_FSM_READY);      
    //End of avoiding
    bleQueue_g.data_idx_tail = 0;
    bleQueue_g.sentSuccess = false;
    PoscvtCfg_g.sendPRF = true;
    set_SysEvent(SYSEVT_DIS_CONNECT);
    PoscvtCfg_g.ble_prf = false;
  }
  else if(PoscvtCfg_g.ble_cancel){
    PoscvtCfg_g.sendDIS = true;
    set_SysEvent(SYSEVT_DIS_CONNECT);
    PoscvtCfg_g.ble_cancel = false;
  }
  else if(PoscvtCfg_g.ble_qr){
    nd_erase_flash(1);                             //erasing a page is 4kBytes
    sprintf((char*)tx_data, "$QRS");
    send_responseToBLEinEvent((char *)tx_data);
    PoscvtCfg_g.ble_qr = false;
  }
  else if(PoscvtCfg_g.ble_qd){
    sprintf((char*)tx_data, "$SUC");
    send_responseToBLEinEvent((char *)tx_data);
    set_SysEvent(SYSEVT_WRITE_QRDATA);
    PoscvtCfg_g.ble_qd = false;
  }
  else if(PoscvtCfg_g.ble_rd){
    if(PoscvtCfg_g.ble_wr_rd){
      set_SysEvent(SYSEVT_WRITE_REVDATA);
      PoscvtCfg_g.ble_wr_rd = false;
    }
    sprintf((char*)tx_data, "$SRD");
    send_responseToBLEinEvent((char *)tx_data);
    PoscvtCfg_g.ble_rd = false;
  }
  else if(PoscvtCfg_g.ble_opt){
    bleQueue_g.sentSuccess = true;
    nd_erase_flash_opt(1);                             //erasing a page is 4kBytes
    resetOPTWriteTimer();
    set_SysEvent(SYSEVT_WRITE_OPT);
    PoscvtCfg_g.ble_opt = false;
  }
  else if(PoscvtCfg_g.ble_cancel){
    PoscvtCfg_g.sendDIS = true;
    set_SysEvent(SYSEVT_DIS_CONNECT);
    PoscvtCfg_g.ble_cancel = false;
  }
  else if(PoscvtCfg_g.ble_qr){
    nd_erase_flash(1);                             //erasing a page is 4kBytes
    sprintf((char*)tx_data, "$QRS");
    send_responseToBLEinEvent((char *)tx_data);
    PoscvtCfg_g.ble_qr = false;
  }
  else if(PoscvtCfg_g.ble_cpk){
    sprintf((char*)tx_data, "CPK#");
    send_responseToBLEinEvent((char *)tx_data);
    PoscvtCfg_g.ble_cpk = false;
  }
  else if(PoscvtCfg_g.ble_tpk){
    sprintf((char*)tx_data, "TPK#");
    send_responseToBLEinEvent((char *)tx_data);
    PoscvtCfg_g.ble_tpk = false;
  }
  else if(PoscvtCfg_g.ble_prs1){ // PRINT
    print_pass_flag = false; 
		
    set_SysEvent(SYSEVT_PRINT_RECEIPT);
     if(PoscvtCfg_g.mode == CFG_MODE_CAT || PoscvtCfg_g.mode == CFG_MODE_KEYPAD){
        PoscvtCfg_g.sendPRS = true;
        set_SysEvent(SYSEVT_PLAY_SOUND);
        if(PoscvtCfg_g.mode == CFG_MODE_KEYPAD){
            set_print_baud_rate(PoscvtCfg_g.baudrate);
            SEGGER_RTT_printf(0, "\r\nKeyPad BaudRate: 0x%x",PoscvtCfg_g.baudrate);
        }
    }
    else{
        clear_print_tick();
        set_SysEvent(SYSEVT_SEND_RECEIPT);
    }
    PoscvtCfg_g.ble_prs1 = false;
  }
  else if(PoscvtCfg_g.ble_prs0){ // NO PRINT 
    //SEGGER_RTT_printf(0, "\r\n%d PRINT PASS SOUND",get_tickcount());
    print_pass_flag = true;
		
    if(PoscvtCfg_g.mode == CFG_MODE_CAT || PoscvtCfg_g.mode == CFG_MODE_KEYPAD){
        PoscvtCfg_g.sendPRS = true;
        set_SysEvent(SYSEVT_PLAY_SOUND);
    }
    else{
        clear_print_tick();
        set_SysEvent(SYSEVT_SEND_RECEIPT);
    }
    set_SysEvent(SYSEVT_CLEAR_RECEIPT);
    set_SysEvent(SYSEVT_DIS_CONNECT);
    PoscvtCfg_g.ble_prs0 = false;
  }
  else if(PoscvtCfg_g.ble_print){
    set_SysEvent(SYSEVT_PRINT_RECEIPT);
    PoscvtCfg_g.ble_print = false;
  }
#ifdef ENABLE_SOA_DFU
  else if(PoscvtCfg_g.ble_fum){    
    PoscvtCfg_g.ble_fum = false;
    PoscvtCfg_g.soa_dfu = true;
    set_SysEvent(SYSEVT_DIS_CONNECT);
  }
#endif
  if(func_over_time(Tickcounter_g.led_tick, 50) == true)                   //500mSec마다 실행, 50 is 500mSec
  {
    Tickcounter_g.led_tick = get_tickcount();
    ctrlLEDState();
    nrf_gpio_pin_toggle(DEF_GPIO_SMDLED_D1);                             //on board SMD LED    
    if(nrf_gpio_pin_read(DEF_GPIO_BUTTON) == 1)
    {
      resetOPTWriteTimer();
      clr_SysEvent(SYSEVT_BTN_PUSH);
    }
  }
  
	if (segment_wait_cnt++ > 200)
	{
		segment_wait_cnt = 0;
		int payment = 0;
		char tx_data[9];
		int len=0;
		
	//if(func_over_time(Tickcounter_g.segment_tick, 1) == true)                   //10mSec마다 실행, 1 is 10mSec
  //{
    //Tickcounter_g.segment_tick = get_tickcount();
		//SEGGER_RTT_printf(0, "\n%d segment_position = %d ",get_tickcount() , segment_position);
		
		//sprintf(tx_data, "%d",pos_parser_get_payment());
		//len = sprintf(tx_data, "%d",5600);
		
//		payment = 5600;
//		
//		do{
//			payment = payment/10;
//			len++;
//		}while(payment > 0);
//		SEGGER_RTT_printf(0, "\n%d len = %d ",get_tickcount() , len);
//		
//		
		switch(segment_position)
		{
			case 0:
				//HC595write(0x01,0x01);
				HC595write(0,0); // clear
				HC595write(0x01,segnum[1]);
				segment_position=1;			
				break;
			case 1:
				//HC595write(0x02,0x02);
				HC595write(0,0); // clear
				HC595write(0x02,segnum[2]);
				segment_position=2;
				break;
			case 2:
				//HC595write(0x04,0x04);
				HC595write(0,0); // clear
				HC595write(0x04,segnum[3]);
				segment_position=3;
				break;
			case 3:
				HC595write(0,0); // clear
				HC595write(0x08,segnum[4]);
				segment_position=4;
				break;
			case 4:
				HC595write(0,0); // clear
				//SEGGER_RTT_printf(0, "\n%d payment/1000 = %d ",get_tickcount() , payment/1000);
				HC595write(0x10,segnum[5]);
				segment_position=5;
				break;
			case 5:
				HC595write(0,0); // clear
				HC595write(0x20,segnum[6]);
				segment_position=6;
				break;
			case 6:
				HC595write(0,0); // clear
				HC595write(0x40,segnum[7]);
				segment_position=7;
				break;
			case 7:
				HC595write(0,0); // clear
				HC595write(0x80,segnum[8]);
				segment_position=0;
				break;
			case 8:
				HC595write(0,0); // clear
				segment_position=0;
				break;
			
			default:
				break;
		}
		//nrf_delay_us(10);
	}
	
  if(sysevent_g & SYSEVT_SEND_BLEDATA)           // Priority Highest 1
  {
    if(send_queuedQRData() == true)
    {
      TAGGING_SOUND; // BLE Auto cut off 기능추가 2020.02.07 ksd
      SEGGER_RTT_printf(0, "\r\n%d TAGGING_SOUND",get_tickcount());
      clr_SysEvent(SYSEVT_SEND_BLEDATA);
    }
  }
  else if(sysevent_g & SYSEVT_SEND_RECEIPT){     // Priority 2  Send Receipt to App
    if(elaspe_times == 0)
      elaspe_times = get_tickcount();
    if(func_over_time(Tickcounter_g.ble_wait_tick,1) == true){
      if(send_receiptData() == true){
          clr_SysEvent(SYSEVT_SEND_RECEIPT);
          if(PoscvtCfg_g.sendPRS){
              PoscvtCfg_g.sendPRS = false;
              send_PRS();
          }
      }
      Tickcounter_g.ble_wait_tick = get_tickcount();
    }
    resetRemainConnect();
  }
  else if(sysevent_g & SYSEVT_WRITE_REVDATA)     // Priority 3   Receive Receipt from App
  {
    resetRemainConnect();
    writeReverseToSRAM();
    clr_SysEvent(SYSEVT_WRITE_REVDATA);
  }
  else if(sysevent_g & SYSEVT_WRITE_QRDATA)                              //QR데이터를 저장
  {
    uint8_t txData[2];
    uint16_t eep_addr;
    volatile uint32_t* pFlagAddress = (uint32_t*)(FLASH_QRCODE_ADDR);
    uint32_t lenToWrite, qrSize;

    if(func_over_time(Tickcounter_g.qr_write_tick,10))                    //100mSec를 기다리는 이유는 flash erase를 기다리는 조건
    {
      Tickcounter_g.qr_write_tick = 0;
      resetRemainConnect();
      //Write QR header
      qrSize = (uint32_t)bleQueue_g.QRData.idx_hdr;
      lenToWrite = 4;
      nd_write_flash((uint32_t*)pFlagAddress, &qrSize, lenToWrite);

      txData[0] = (uint8_t)(bleQueue_g.QRData.idx_hdr>>8);
      txData[1] = (uint8_t)bleQueue_g.QRData.idx_hdr;
      eep_addr = EEP_QRINFO_ADDR;
      eep_write_array(eep_addr, txData, 2);          //QR data info
      while(IsSPIBusy());
      //Write QR data
      pFlagAddress = (uint32_t*)(FLASH_QRCODE_ADDR + 4U);          //4 is header, length of QR code
      lenToWrite = (uint32_t)(bleQueue_g.QRData.idx_hdr % 4);
      if(lenToWrite != 0) 
          lenToWrite = (uint32_t)bleQueue_g.QRData.idx_hdr + (4 - lenToWrite);
      else 
          lenToWrite = (uint32_t)bleQueue_g.QRData.idx_hdr;
      //NRF_WDT->RR[0] = WDT_RR_RR_Reload;            //clear watchdog
      nd_write_flash((uint32_t*)pFlagAddress, (uint32_t const *)&bleQueue_g.QRData.data, lenToWrite);
      nrf_delay_ms(1 + lenToWrite/4);
      eep_addr = EEP_QRDATA_ADDR;
      eep_write_array(eep_addr, bleQueue_g.QRData.data, bleQueue_g.QRData.idx_hdr);          //3 is command("QD$") 
      while(IsSPIBusy());
      clr_SysEvent(SYSEVT_WRITE_QRDATA);
      NORMAL_SOUND;
    }
  }
  else if(sysevent_g & SYSEVT_WRITE_OPT)                                        //옵션을 메모리에 저장해야하는 경우
  {
    if(func_over_time(Tickcounter_g.button_tick, 10))                         //100mSec를 기다렸다가 메모리에 저장 DataSheet 11.8.1   
    {
      resetOPTWriteTimer();
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

      set_print_baud_rate(PoscvtCfg_g.baudrate);
      print_system_prameter(true);
      if(PoscvtCfg_g.buttonEvent == true && PoscvtCfg_g.mode == CFG_MODE_KEYPAD){
          set_print_baud_rate(PoscvtCfg_g.keypad_baudrate);
        SEGGER_RTT_printf(0, "\r\n Keypad BaudRate : 0x%d",PoscvtCfg_g.keypad_baudrate);
      }
      PoscvtCfg_g.buttonEvent = false;
      clr_SysEvent(SYSEVT_WRITE_OPT);
      NORMAL_SOUND;
    }
  } 
  else if(sysevent_g & SYSEVT_PRINT_RECEIPT){
    if(new_print_receipt(true) == 0){
        set_SysEvent(SYSEVT_CLEAR_RECEIPT);
    }
    set_SysEvent(SYSEVT_DIS_CONNECT);
    clr_SysEvent(SYSEVT_PRINT_RECEIPT);
  }
  else if(sysevent_g & SYSEVT_PLAY_SOUND){        // Save Sound Index
    //BLE Auto cut off 기능추가 2020.02.07 ksd
    if (last_sound == true)
    {
      play_sound_set_total_value(sndToTal_g , true);
    }
    else
    {
      play_sound_set_total_value(sndToTal_g , false);
      last_sound = true;
    }

    clr_SysEvent(SYSEVT_PLAY_SOUND);
  }
  else if(sysevent_g & SYSEVT_DIS_CONNECT){
    resetRemainConnect();
    process_wait_complete_packet();
    if(PoscvtCfg_g.sendDIS){
        PoscvtCfg_g.sendDIS = false;
        send_DIS();
        SEGGER_RTT_printf(0, "\r\n%d [MCU->APP] DIS",get_tickcount());
    }
    if(PoscvtCfg_g.sendPRS){
        PoscvtCfg_g.sendPRS = false;
        send_PRS();
        SEGGER_RTT_printf(0, "\r\n%d [MCU->APP] PRS",get_tickcount());
    }
    if(PoscvtCfg_g.sendPRF){
        PoscvtCfg_g.sendPRF = false;
        send_PRF();
        SEGGER_RTT_printf(0, "\r\n%d [MCU->APP] PRF",get_tickcount());
				
        //BLE Auto cut off 기능추가 2020.02.07 ksd
        prf_cancel_flag = true;
    }
#ifdef ENABLE_SOA_DFU
    if(PoscvtCfg_g.soa_dfu){
        send_FUM();
        SEGGER_RTT_printf(0, "\r\n%d [MCU->APP] FUM",get_tickcount());
    }else 
#endif    
    if(bleQueue_g.sentSuccess == false){                                  //영수증 데이터가 처리된 경우
      if (no_receipt_flag)
      {
        no_receipt_flag = false;
        SEGGER_RTT_printf(0, "\r\n%d NO_RECEIPT_SOUND",get_tickcount());
        NO_RECEIPT_SOUND;
      }
      else if (prf_cancel_flag)
      {
         prf_cancel_flag = false;
         SEGGER_RTT_printf(0, "\r\n%d CANCEL_SOUND",get_tickcount());
			
         if(PoscvtCfg_g.mode == CFG_MODE_POS || PoscvtCfg_g.mode == CFG_MODE_CAT)
         {
           CANCEL_SOUND;
         }
         else if(PoscvtCfg_g.mode == CFG_MODE_KEYPAD)
         {
           if (*sndToTal_g == NULL) //키패드 수신값을 보고 null 유무로 확인
           {
             NO_RECEIPT_SOUND;
           }
           else
           {
             *sndToTal_g = NULL;
             CANCEL_SOUND;
           }
         }
       }
       else
       {
         SEGGER_RTT_printf(0, "\r\n%d ERROR_SOUND",get_tickcount());
        ERROR_SOUND;
       }
			
    }
    clr_SysEvent(SYSEVT_DIS_CONNECT);
    clr_SysEvent(SYSEVT_TIMEOUTSTART);
    set_SysEvent(SYSEVT_BLE_DISCONNECT);
    Tickcounter_g.ble_disconect_wait = get_tickcount();
  }
  else if(sysevent_g & SYSEVT_CLEAR_RECEIPT){     //영수증 데이터를 지울 때 실행
    elaspe_times = 0;
    pos_parser_reset();
    clear_receipt();
    receiptFSM(RECEIPT_FSM_READY);
    clr_SysEvent(SYSEVT_CLEAR_RECEIPT);
    if(PoscvtCfg_g.mode == CFG_MODE_KEYPAD){
        set_print_baud_rate(PoscvtCfg_g.keypad_baudrate);
        SEGGER_RTT_printf(0, "\r\nKeyPad 9600bps");
        sendCAT2Succeed();
    }
  }  
  else if(sysevent_g & SYSEVT_BLE_DISCONNECT){
    if(func_over_time(Tickcounter_g.ble_disconect_wait,30)){ // 300ms
        nd_disconnect();
        clr_SysEvent(SYSEVT_BLE_DISCONNECT);
        SEGGER_RTT_printf(0, RTT_CTRL_TEXT_RED"\r\n BLE Disconnect\n"RTT_CTRL_RESET);
#ifdef ENABLE_SOA_DFU
        if(PoscvtCfg_g.soa_dfu){
            PoscvtCfg_g.soa_dfu = false;
            if(func_enter_bootloader() == NRF_SUCCESS){
                SEGGER_RTT_printf(0, RTT_CTRL_TEXT_BLUE"\r\n Enter Bootloader for DFU!!\n"RTT_CTRL_RESET);
                NVIC_SystemReset();
            }
            else{
                SEGGER_RTT_printf(0, "\r\nRequest to enter bootloader mode failed asynchroneously.");
            }
        }
#endif
    }    
  }
  else if(sysevent_g & SYSEVT_TIMEOUTSTART)                                  //BLE연결 timeout이 시작되었으면 timeout값 확인
  {
    if(process_is_there_packet() == 0 && (sysevent_g & 0x007F) == 0){
      checkRemainConnect();
    }
  }
  
  if(new_print_receipt(false) == 3){
    set_SysEvent(SYSEVT_CLEAR_RECEIPT);
  }
	
  (void)play_sound();

  if(sysevent_g & SYSEVT_ADV_START){
    if((sysevent_g & SYSEVT_TIMEOUTSTART) == 0){
        if(running_advertising == false){
        SEGGER_RTT_printf(0, "\r\n Start Advertising.");
        advertising_start();
        running_advertising = true;
            
        }
        clr_SysEvent(SYSEVT_ADV_START);
    }
    else{
    clr_SysEvent(SYSEVT_ADV_START);
  }
  }
  if(sysevent_g & SYSEVT_ADV_STOP){
    if((sysevent_g & SYSEVT_TIMEOUTSTART) == 0){
        if(running_advertising == true){
            if(IsAdvertised()){
        SEGGER_RTT_printf(0, "\r\n Stop Advertising.");
        advertising_stop();
        running_advertising = false;
                clr_SysEvent(SYSEVT_ADV_STOP);
            }
        }
        else{
            clr_SysEvent(SYSEVT_ADV_STOP);
        }
    }
    else{
        clr_SysEvent(SYSEVT_ADV_STOP);
    }
  }
  if(sysevent_g & SYSEVT_FORCE_PRINT){
      bleQueue_g.sentSuccess = true;
      if(new_print_receipt(true) == 0){
        set_SysEvent(SYSEVT_CLEAR_RECEIPT);
      }
      clr_SysEvent(SYSEVT_FORCE_PRINT);
  }

  if((sysevent_g & ~(SYSEVT_BLINK_LED | SYSEVT_BTN_PUSH)) != 0){
    clr_SysEvent(SYSEVT_BTN_PUSH);
  }

  if(sysevent_g & SYSEVT_BTN_PUSH)                                              //버튼이벤트 발생시 확인
  {
    if(PoscvtCfg_g.IsReceiptdata == false)
    {
      if(func_over_time(Tickcounter_g.button_tick,300))                     //버튼이벤트가 3초동안 유지된 경우 모드변경됨
      {
        resetOPTWriteTimer();

        if(PoscvtCfg_g.mode == CFG_MODE_POS)
        {
          PoscvtCfg_g.mode = CFG_MODE_CAT;
        }
        else if(PoscvtCfg_g.mode == CFG_MODE_CAT)
        {
          PoscvtCfg_g.mode = CFG_MODE_KEYPAD;
        }
        else
        {
          PoscvtCfg_g.mode = CFG_MODE_POS;
        }
        PoscvtCfg_g.buttonEvent = true;
        nrf_gpio_pin_set(DEF_GPIO_RED1_PWM);                                    //Off LED
        nd_erase_flash_opt(1);
        resetOPTWriteTimer();                                                       //Flash를 지우기 위한 마진(딜레이)
      
        play_sound_mode(PoscvtCfg_g.mode);

        set_SysEvent(SYSEVT_WRITE_OPT);
        clr_SysEvent(SYSEVT_BTN_PUSH);
      }
    }
    else if((sysevent_g & ~(SYSEVT_BLINK_LED | SYSEVT_BTN_PUSH)) == 0)
    {
      bleQueue_g.sentSuccess = true;
      if(new_print_receipt(true) == 0){
        set_SysEvent(SYSEVT_CLEAR_RECEIPT);
      }
      clr_SysEvent(SYSEVT_BTN_PUSH);
    }
  }
}

/*
*@brief : 총합금액을 출력하기 위해 버퍼에 저장
*/
void setSoundTotal(uint32_t value)
{
  sprintf((char*)sndToTal_g, "%d", value);
}

/*
*@breif : set last flash info
*/
void setLastFalshInfo(uint32_t address, uint32_t* pBuff)
{
  //int i;
  
  //LastFlashInfo_g.address = address;
  //for(i=0; i<24; ++i) LastFlashInfo_g.buff[i] = pBuff[i];
} 

/*
* @brief : initi fstorage
*/
void init_fstorage(void)
{
  p_fs_api_g = &nrf_fstorage_sd;          //using softdevice

  nrf_fstorage_init(
        &fstorage,       /* You fstorage instance, previously defined. */
        p_fs_api_g,        /* Name of the backend. */
        NULL                /* Optional parameter, backend-dependant. */
    );
}

/*
* @brief : fstorage handler
*/
static void fstorage_handler(nrf_fstorage_evt_t * p_evt)
{
  switch (p_evt->id)
  {
      case NRF_FSTORAGE_EVT_WRITE_RESULT:
        //SEGGER_RTT_printf(0, "\nWrited");
        break;

      case NRF_FSTORAGE_EVT_ERASE_RESULT:
        //SEGGER_RTT_printf(0, "\nErased");
        break;

      default:
        break;
  }  
}

void wait_for_flash_ready(nrf_fstorage_t const * p_fstorage)
{
    /* While fstorage is busy, sleep and wait for an event. */
    while (nrf_fstorage_is_busy(p_fstorage))
    {
      nrf_delay_ms(1);
    }
}

/*
*@brief : erase flash
*/
bool nd_erase_flash_opt(uint32_t NoOfPages)
{
  bool ret = false;
  
  ret_code_t rc = nrf_fstorage_erase(
      &fstorage,   /* The instance to use. */
      FLASH_OPTION_ADDR,     /* The address of the flash pages to erase. */
      NoOfPages, /* The number of pages to erase. */
      NULL            /* Optional parameter, backend-dependent. */
  );
  
  if (rc == NRF_SUCCESS)
  {
      /* The operation was accepted.
         Upon completion, the NRF_FSTORAGE_ERASE_RESULT event
         is sent to the callback function registered by the instance. */
    //SEGGER_RTT_printf(0, "Erase succeed\n");
    
    ret = true;
  }
  else
  {
      /* Handle error.*/
    //SEGGER_RTT_printf(0, "Erase failed\n");
        
    ret = false;
  }
  
  return ret;
}

/*
*@brief : erase flash
*/
bool nd_erase_flash(uint32_t NoOfPages)
{
  bool ret = false;
  
  ret_code_t rc = nrf_fstorage_erase(
      &fstorage,   /* The instance to use. */
      FLASH_QRCODE_ADDR,     /* The address of the flash pages to erase. */
      NoOfPages, /* The number of pages to erase. */
      NULL            /* Optional parameter, backend-dependent. */
  );
  
  if (rc == NRF_SUCCESS)
  {
      /* The operation was accepted.
         Upon completion, the NRF_FSTORAGE_ERASE_RESULT event
         is sent to the callback function registered by the instance. */
    //SEGGER_RTT_printf(0, "\nQR Erase succeed");
    
    ret = true;
  }
  else
  {
      /* Handle error.*/
    //SEGGER_RTT_printf(0, "\nQR Erase failed");
        
    ret = false;
  }
  
  return ret;
}

/*
*@brief : write flash
*/
bool nd_write_flash(uint32_t * p_dst, uint32_t const * p_src, uint32_t size)
{
  ret_code_t result;
  bool ret;

  result = nrf_fstorage_write(&fstorage, (uint32_t)p_dst, p_src, size, NULL);
  if(result == NRF_SUCCESS) 
  {
    ret = true;
  }
  else 
  {
    ret = false;
  }

  return ret;
}

/*
* @brief : set system evetn
*/
void set_SysEvent(uint16_t event)
{
  sysevent_g |= event;
}

/*
* @brief : clear system event
*/
void clr_SysEvent(uint16_t event)
{
  sysevent_g &= (~event);
}

/*
* @brief : initialize parameters for stick
*/
void init_stick_var(void)
{
  uint32_t flashAddr = FLASH_OPTION_ADDR;
  uint32_t flashData = 0;

  bTimer_handled_g = false;

  //각 timeout값 초기화
  Tickcounter_g.led_tick = 0;
  Tickcounter_g.button_tick = 0;
  Tickcounter_g.reamin_conn_tick = 0;
  Tickcounter_g.qr_write_tick = 0;
  Tickcounter_g.ble_disconect_wait = 0;
	Tickcounter_g.segment_tick = 0;
  //End of timeout 초기화

  //set pos mode
  flashData = nd_nvmc_read(flashAddr);  
  if(flashData == 0xFFFFFFFFU)                          //데이터가 없는  경우
  {
    PoscvtCfg_g.volume = 16;
    PoscvtCfg_g.baudrate = CFG_BAUD_9600;
    PoscvtCfg_g.mode = CFG_MODE_POS;
    PoscvtCfg_g.print_type = CFG_PRINT_A;
  }
  else
  {
    PoscvtCfg_g.print_type = (uint8_t)(flashData >> 24);
    PoscvtCfg_g.volume = (uint8_t)(flashData >> 16);
    PoscvtCfg_g.baudrate = (uint8_t)(flashData >> 8);
    PoscvtCfg_g.mode = (uint8_t)(flashData);
  }
#ifdef POS_PARSER_ENABLE
  pos_parser_load_cpk_tpk();
#endif
#if 0
  switch(PoscvtCfg_g.baudrate)
  {
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
    break;
  }

  SoundVol(PoscvtCfg_g.volume);  
  setCVTMode(PoscvtCfg_g.mode);
  setPrinttype(PoscvtCfg_g.print_type);
#endif
  SoundVol(PoscvtCfg_g.volume);  
  PoscvtCfg_g.IsReceiptdata = false;    
  PoscvtCfg_g.IsTagReceived = false;
  PoscvtCfg_g.sendDIS = false;
  PoscvtCfg_g.sendPRS = false;
  PoscvtCfg_g.keypad_baudrate = CFG_BAUD_9600;
  setReverseData(0, 0, true);           //to clear reversedata buffer
  
  PoscvtCfg_g.auto_cut = false;
  sys_tickcount_g = 0;
}

/*
*@ brief : 프린트 타입 저장
*/
void set_print_mode(uint8_t value)
{
  PoscvtCfg_g.print_type = value;

  switch(value)
  {
  case CFG_PRINT_A:
    nrf_gpio_pin_set(RS232_IN1);
    nrf_gpio_pin_clear(POS_DIR);
    //SEGGER_RTT_printf(0, "\nsetPrinttype : A");
    break;
    
  case CFG_PRINT_B:
    nrf_gpio_pin_clear(RS232_IN1);
    nrf_gpio_pin_set(POS_DIR);
    //SEGGER_RTT_printf(0, "\nsetPrinttype : B");
    break;
    
  case CFG_PRINT_C:
    //SEGGER_RTT_printf(0, "\nsetPrinttype : C");
    break;
    
  default:
    PoscvtCfg_g.print_type = CFG_PRINT_A;
    break;
  }
}

/*
*@ brief : 통신속도 설정
*/
void set_print_baud_rate(uint32_t value)
{
  switch(value)
  {
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
#ifdef SPECIAL_BAUDRATE_57600
  {
    unsigned int special_rate = pos_parser_get_baudrate();
    if(special_rate >= 57000 && special_rate <= 58000){
        uart_setBaudrate(special_rate);
    }
    else{
    uart_setBaudrate(57600);
    }
  }
#else
    uart_setBaudrate(57600);
#endif
    break;
    
  case CFG_BAUD_115200:
    uart_setBaudrate(115200);
    break;
    
  default:
    PoscvtCfg_g.baudrate = CFG_BAUD_9600;
    uart_setBaudrate(9600);
    break;
  }
}

/*
*@ brief : 사운드 볼륨 변경
*/
void setVolume(uint32_t value)
{
  if(value > 4) value = 4;
  
  PoscvtCfg_g.volume = value * 4;

  if(PoscvtCfg_g.volume > 16) PoscvtCfg_g.volume = 16;
  PoscvtCfg_g.volume = 16 - PoscvtCfg_g.volume;
  
  //SEGGER_RTT_printf(0, "\nVol : %d", PoscvtCfg_g.volume );
  SoundVol(PoscvtCfg_g.volume);
}

/*
*@ brief : CVT 모드
*/
void play_sound_mode(uint32_t value)
{
  PoscvtCfg_g.mode = value;
      
  switch(value)
  {
  case CFG_MODE_POS:
    POS_MODE_SOUND;
    break;

  case CFG_MODE_CAT:
    CARD_MODE_SOUND;
    break;
    
  case CFG_MODE_KEYPAD:
    KEYPAD_MODE_SOUND;
    break;
    
  default:
    PoscvtCfg_g.mode = CFG_MODE_POS;
    POS_MODE_SOUND;
    break;
  }
}

/*
* @brief : tick counter, 10mSec
*/
uint32_t get_tickcount(void)
{
  return sys_tickcount_g;
}

/*
* @brief : call IMU function every 10mSec
*/
void cb_tick_handler(void)
{
  ++sys_tickcount_g;
}

/*
* @breif : deactive gyro&mag sensor
*/
void sleep_sensors(void)
{
}

/*
*@ brief : GPIOTE 설정
*/
static void sf_gpiote_handler(nrf_drv_gpiote_pin_t pin, nrf_gpiote_polarity_t action)
{
  SEGGER_RTT_printf(0, "> gpiote interrupt\n");
  
  switch(pin)
  {
  case DEF_GPIO_BUTTON:
    resetOPTWriteTimer();            //reset tickcout of button
    set_SysEvent(SYSEVT_BTN_PUSH);
    break;
    
  default:
    //Do nothing
    break;
  }
}

/*
*@ brief : GPIOTE 초기화
*/
void gpiote_init(void)
{
     /* config interrupt */
    nrf_drv_gpiote_in_config_t config = GPIOTE_CONFIG_IN_SENSE_HITOLO(false);
    config.pull = NRF_GPIO_PIN_PULLUP;
		SEGGER_RTT_printf(0, "\n%d DEBUG MESSAGE",get_tickcount());
    if(!nrf_drv_gpiote_is_init()) {
			SEGGER_RTT_printf(0, "\n%d DEBUG MESSAGE",get_tickcount());
			nrf_drv_gpiote_init();
		}
		SEGGER_RTT_printf(0, "\n%d DEBUG MESSAGE",get_tickcount());
		
    nrf_drv_gpiote_in_init(DEF_GPIO_BUTTON, &config, sf_gpiote_handler);
    
    nrf_drv_gpiote_in_event_enable(DEF_GPIO_BUTTON, true);
}

void set_advertising_active(void)
{
    running_advertising = true;
}

void clear_advertising_active(void)
{
    running_advertising = false;
}

/*
*@brief : OPT처리 timeout 리셋
*/
void resetOPTWriteTimer(void)
{
  Tickcounter_g.button_tick = get_tickcount();
}

void clear_print_tick(void)
{
    Tickcounter_g.ble_wait_tick = 0;
}


uint32_t func_enter_bootloader(void)
{
    uint32_t err_code;
    err_code = sd_power_gpregret_clr(0, 0xffffffff);
    VERIFY_SUCCESS(err_code);

    err_code = sd_power_gpregret_set(0, BOOTLOADER_DFU_START);
    VERIFY_SUCCESS(err_code);
    //NRF_POWER->GPREGRET = 0xB1;    
    return NRF_SUCCESS;
}
