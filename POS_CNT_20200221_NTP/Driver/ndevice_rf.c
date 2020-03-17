#include "ndevice_rf.h"

static stRFtask* pRFtask_g = ((stRFtask*)ND_RF_TASK);
static stRFinterrupt* pRFinterrupt_g = ((stRFinterrupt*)ND_RF_INTERRUPT);
static stRFpacket* pRFpacket_g = ((stRFpacket*)ND_RF_PACK_CFG);

static uint8_t rf_adv_buffer_g[128+1];
static stRXBuffer rf_rx_buffer_g;

stPacketParser rfPacketParser_g;

fnRF_RCVHandler RCVHandler_g;

//uint8_t ble_channel[40] = {4, 6, 8, 10, 12, 14, 16, 18, 20, 22, 24, 28, 30, 32, 34, 36, 38, 40, 42, 44, 46, 48, 50, 52, 54, 56, 58, 60, 62, 64, 66, 68, 70, 72, 74, 76, 78, 2, 26, 80,};

void nd_rf_switch_mode(uint8_t mode);
static void nd_rf_blocked_wait_dis(void);

static int8_t ch2freq(uint8_t ch)
{
	/* nRF51 Series Reference Manual v2.1, section 16.2.19, page 91
	 * Link Layer specification section 1.4.1, Core 4.1, page 2502
	 *
	 * The nRF51822 is configured using the frequency offset from
	 * 2400 MHz.
	 */
	switch (ch) {
	case 37:
		return 2;
	case 38:
		return 26;
	case 39:
		return 80;
	default:
		if (ch > 39)
			return -1;
		else if (ch < 11)
			return 4 + (2 * ch);
		else
			return 6 + (2 * ch);
	}
}

/*
* @brief : Initialize Ndevice RF driver
*/
void nd_rf_init(fnRF_RCVHandler mainRCVHandler)
{
  ND_RF_POWER = 1;              //Radio Enalbed
  
  if(nd_rf_get_rfstatus() != ND_RF_STATE_DISABLED)
  {
    nd_rf_disable_RF();
    
    while(nd_rf_get_rfstatus() != ND_RF_STATE_DISABLED) {}
  }
  
  //Set HFCLK register
  
  pRFpacket_g->txpower = ND_RF_TXPWR_p4dBm;
  //pRFpacket_g->mode = ND_RF_MODE_BLE1M;
  pRFpacket_g->mode = ND_RF_MODE_NRF1M;

  ND_RF_MODECNF0 = ND_RF_MODECNF0_CENTER | ND_RF_MODECNF0_FASTMODE;
  
  // NRF_RADIO->PREFIX0 = 0x0000008e;
  // NRF_RADIO->BASE0 = 0x89bed600;
  //Must change to randomized address
  pRFpacket_g->base0 = (ADV_CHANNEL_ADDR<< 8UL) & (0xFFFFFF00UL);
  pRFpacket_g->prefix0 = 0x0000008e;
  pRFpacket_g->rxaddress = 1UL;
  pRFpacket_g->txaddress = 0UL;           //Using base0 address for TX
  
  pRFpacket_g->pcnf0 = (1UL << 8) | 8UL;               //1 s0 field, Length field is 8
  pRFpacket_g->pcnf1 = (1UL << 25) | (3UL << 16) | (128UL);         //Data whiten is enabled, Base address length is 3, Max length is 128
  
  pRFpacket_g->crccnf = (0x00000001UL << 8) | 3;              //Using 3 bytes of CRC and no include address field, it is BLE packet
  pRFpacket_g->crcinit = 0x00555555;
  pRFpacket_g->crcpoly = 0x100065B;
  pRFpacket_g->tifs = 145;
  //Set PDU

  //Interrupt enabled
  //ND_RF_SHORTS = ND_RF_SHORTS_RDST | ND_RF_SHORTS_EDDI;
  ND_RF_SHORTS = ND_RF_SHORTS_RDST | ND_RF_SHORTS_EDDI;

  pRFinterrupt_g->intenset = ND_RF_INT_END;

  nrf_drv_common_irq_enable(RADIO_IRQn, 7);

  nd_rf_set_packet();
  
  //initialize variables
  rf_rx_buffer_g.idx_header = 0;
  rf_rx_buffer_g.idx_tail = 0;
  
  RCVHandler_g = mainRCVHandler;
}

/*
* @brief : set packet
*/
void nd_rf_set_packet(void)
{
  uint32_t i;
  uint8_t frame_size = 0;
  static uint8_t device_name[] = "(c)HYOIL";
 
  //Packet header[1byte] : RxAddrType[7], TxAddrType[6], RFU[5:4], Adv type[3:0](ADV_NONCONN_IND) -> 0x42
  //Pachet length[1byte] : RFU[7:6], using 6bit, 6~37 available -> 0x16
  //adv address[6 bytes] : 0xAA, 0xBB, 0xCC, 0xDD, 0xEE, 0xFF

  //Set packet header
  rf_adv_buffer_g[frame_size++] = 0x42;
  
  //Set packet length(6~37 available)
  rf_adv_buffer_g[frame_size++] = 37;

  //Set device address 6 bytes
  for(i=0; i<6; ++i) rf_adv_buffer_g[i+2] = nd_get_randomed_num();
  frame_size += 6;

  //Start of Flags AD
  rf_adv_buffer_g[frame_size++] = 0x02;         //Number of Flags
  rf_adv_buffer_g[frame_size++] = 0x01;         //Flags AD type
  
  //Flags value
  /*
   bit 0 (OFF) LE Limited Discoverable Mode
   bit 1 (OFF) LE General Discoverable Mode
   bit 2 (ON) BR/EDR Not Supported
   bit 3 (OFF) Simultaneous LE and BR/EDR to Same Device Capable (controller)
   bit 4 (OFF) Simultaneous LE and BR/EDR to Same Device Capable (Host)  
  */
  rf_adv_buffer_g[frame_size++] = 0x04;                   //BR/EDR Not Supported
  //End of Flags AD
  
  //Start of Local name
  rf_adv_buffer_g[frame_size++] = 0x09;                   //Number of bytes that follow in the first AD Structure
  rf_adv_buffer_g[frame_size++] = 0x09;                   //# Complete Local Name AD Type

  for(i=0; i<8; ++i)            //Size of HYOIL
  {
    rf_adv_buffer_g[frame_size+i] = device_name[i];
  }
  //End of local name  
}

/*
* @brief : Advertising channel
*/
char nd_rf_nextchannel(bool reset)
{
  //static char ch = 37;
  const char broad_ch[3] = {37, 38, 39};
  static char ch_index;
  char ch;

  if(ch_index >= 3) ch_index = 0;
  
  if(reset == true) 
  {
    ch_index = nd_get_randomed_num() % 3;
  }
  else
  {
    ch = broad_ch[ch_index++];
  }
  
  return ch;
}

/*
* @brief : Auto schduled using advertising channels
*/
uint8_t nd_rf_autoadvsend(uint8_t *pData, uint8_t len)
{
  uint8_t frame_size = 0;
  uint8_t i;

  //nd_rf_blocked_wait_dis();
  if(nd_rf_get_rfstatus() != ND_RF_STATE_DISABLED) return ND_UTIL_FAILED;

  frame_size = 21;              //21 is 2 for header, 6 for device address, 3 for flasg, 10 for local name

  rf_adv_buffer_g[frame_size++] = len + 1;
  rf_adv_buffer_g[frame_size++] = 0xff;             //manufacturer specific data

  for(i=0; i<len; ++i) rf_adv_buffer_g[frame_size++] = pData[i];

  rf_adv_buffer_g[1] = frame_size;              //Total size of packet

  nd_rf_nextchannel(true);
  nd_rf_autosend_fsm(true);

  return ND_UTIL_SUCCEED;
}

/*
* @brief : send rf data, totally 31 bytes avaialbe
*/
uint8_t nd_rf_advsend(uint8_t *pData, uint8_t len, uint8_t channel)
{
  uint8_t frame_size = 0;
  uint8_t i;

  nd_rf_blocked_wait_dis();

  frame_size = 21;              //18 is 2 for header, 6 for device address, 3 for flasg, 10 for local name

  rf_adv_buffer_g[frame_size++] = len + 1;  
  rf_adv_buffer_g[frame_size++] = 0xff;             //manufacturer specific data
  
  for(i=0; i<len; ++i) rf_adv_buffer_g[frame_size++] = pData[i];

  rf_adv_buffer_g[1] = frame_size;              //Total size of packet

  nd_rf_channel(channel);

  pRFpacket_g->packetptr = (uint32_t)rf_adv_buffer_g;
  
  pRFtask_g->txen = 1;

  return ND_UTIL_SUCCEED;
}

/*
* @brief : sending raw packet
*/
void nd_rf_rawsend(uint8_t *pData, uint8_t len)
{
  uint8_t frame_size = 0;
  uint8_t i;

  nd_rf_blocked_wait_dis();

  rf_adv_buffer_g[11] = 2;
  frame_size = 14;              //18 is 2 for header, 6 for device address, 3 for flasg, 10 for local name

  rf_adv_buffer_g[frame_size++] = len + 1;  
  rf_adv_buffer_g[frame_size++] = 0xff;             //manufacturer specific data

  for(i=0; i<len; ++i) rf_adv_buffer_g[frame_size++] = pData[i];

  rf_adv_buffer_g[1] = frame_size;              //Total size of packet

  nd_rf_nextchannel(true);
  nd_rf_autosend_fsm(true);
  
  pRFpacket_g->packetptr = (uint32_t)rf_adv_buffer_g;
}

/*
* @breif : resend rf data without channel changing
*/
uint8_t nd_rf_resend(void)
{
  pRFpacket_g->packetptr = (uint32_t)rf_adv_buffer_g;

  pRFtask_g->start = 1;

  return ND_UTIL_SUCCEED;
}

static void nd_rf_blocked_wait_dis(void)
{
  if(nd_rf_get_rfstatus() != ND_RF_STATE_DISABLED)
  {
    nd_rf_disable_RF();

    while(nd_rf_get_rfstatus() != ND_RF_STATE_DISABLED) {}
  }  
}

/*
* @brief : set rf channel
*/
void nd_rf_channel(uint8_t ch)
{
  uint8_t freq = ch2freq(ch);

  pRFpacket_g->frequency = freq;                //2.4GHz + 0~100 (MHz)
  pRFpacket_g->datawhieteiv = ch & 0x3F;              // BLE sets the data whitening IV to the same as the frequency. I assume there is a good reason.
}

void nd_rf_disable_RF(void)
{
  pRFtask_g->disable = 1;
}

/*
* @brief : get radio status
*/
uint8_t nd_rf_get_rfstatus(void)
{
  return (uint8_t)(pRFpacket_g->state);
}

/*
* @brief : return state of RF 
*/
uint8_t nd_rf_IsBusy(void)
{
  if(nd_rf_get_rfstatus() != ND_RF_STATE_DISABLED) return true;
  else return false;
}

/*
* @brief : FSM of RF
*/
void nd_rf_autosend_fsm(uint8_t reset)
{
  static uint8_t status = AUTOSEND_READY;
  static uint8_t sent_cnt = 0;
  static uint32_t old_tick;
  static uint8_t timeout = 2;
  const uint8_t tiemout_val = 1;

  if(reset == true)
  {
    status = AUTOSEND_READY;
    
    //To debug
    if(sent_cnt < 3) SEGGER_RTT_printf(0, "Under 3");
    
    sent_cnt = 0;
  }
  
  switch(status)
  {
  case AUTOSEND_READY:
    if(nd_rf_get_rfstatus() == ND_RF_STATE_DISABLED)
    {
      status = AUTOSEND_SEND;

      timeout = tiemout_val;
    }
    break;

  case AUTOSEND_SEND:
    nd_rf_channel(nd_rf_nextchannel(false));
    
    pRFpacket_g->packetptr = (uint32_t)rf_adv_buffer_g;                     //set buffer pointer
    pRFtask_g->txen = 1;

    status = AUTOSEND_WAIT;
    break;

  case AUTOSEND_WAIT:
    if(nd_rf_get_rfstatus() == ND_RF_STATE_DISABLED)
    {
      ++sent_cnt;
      if(sent_cnt >= 3) status = AUTOSEND_RX;
      else status = AUTOSEND_SEND;
    }    
    break;

  case AUTOSEND_RX:
#if 0
    status = AUTOSEND_FINISH;
#else
    if(nd_rf_get_rfstatus() == ND_RF_STATE_DISABLED)
    {
      if(rf_rx_buffer_g.idx_header >= ND_RF_RXBUFFER_SIZE) rf_rx_buffer_g.idx_header = 0;
      pRFpacket_g->packetptr = (uint32_t)rf_rx_buffer_g.data[rf_rx_buffer_g.idx_header];                     //set buffer pointer

      pRFtask_g->rxen = 1;

      old_tick = get_tickcount();
      
      status = AUTOSEND_RCV;
    }
#endif
    break;

  case AUTOSEND_RCV:
    if(nd_rf_get_rfstatus() == ND_RF_STATE_DISABLED)
    {
      //SEGGER_RTT_printf(0, "rcv[0]:%02x, rcv[1]:%02x, rcv[2]:%02x", rf_rx_buffer_g.data[rf_rx_buffer_g.idx_tail][0], rf_rx_buffer_g.data[rf_rx_buffer_g.idx_tail][1], rf_rx_buffer_g.data[rf_rx_buffer_g.idx_tail][2]);
      //++rf_rx_buffer_g.idx_tail;
      //if(rf_rx_buffer_g.idx_tail >= ND_RF_RXBUFFER_SIZE) rf_rx_buffer_g.idx_tail = 0;
      
      ++rf_rx_buffer_g.idx_header;
     
      dbg_nd_rf_rcv();

      status = AUTOSEND_FINISH;
    }
    else                    //Timeout checking
    {
      if(get_tickcount() != old_tick)
      {
        old_tick = get_tickcount();

        --timeout;
        if(timeout == 0) 
        {
          status = AUTOSEND_FINISH;
          nd_rf_disable_RF();
        }
      }
    }
    break;    
    
  case AUTOSEND_FINISH:
    break;
  }
}

uint8_t packet_parser(uint8_t *pBuff, uint8_t *pLen)
{
  if(pBuff[0] >= 10) *pLen = 10;
  else *pLen = pBuff[0];

  return pBuff[1];
}

/*
@ biref : Data parser for HyoIL
*/
char nd_rf_rcv_parser(uint8_t *pBuff, stPacketParser *pOutResult)
{
  //Compare company name
  //Compare company ID
  //Check broadcasting ID
  //get Command
  
  volatile uint8_t *pCom_name;
  const uint8_t com_name[10 + 1] = "QODE stick";
  uint8_t i;
  uint8_t packet_len, packet_idx;
  uint8_t total_len;
  uint8_t packet_type;

  total_len = 28;
  //SEGGER_RTT_printf(0, "total len:%02x", total_len);

  if(total_len == 0) return -1;

  packet_idx = 8;               //skip header & address
  pOutResult->ComID = 0x0000;
  pOutResult->BroadID = 0;

  while(total_len)
  {
    packet_type = packet_parser(pBuff + packet_idx, &packet_len);
    if(packet_len == 0) break;

    packet_idx += 2;           //skip type index

    switch(packet_type)
    {
    case 0x01:
      pOutResult->Ver = pBuff[packet_idx++];
      break;
    case 0x09:
      pCom_name = (volatile uint8_t*)(pBuff + packet_idx);

      packet_idx += (packet_len-1);
      break;
    case 0xFF:
      pOutResult->ComID = (uint16_t)pBuff[packet_idx++];
      pOutResult->ComID <<= 8;
      pOutResult->ComID |= (uint16_t)pBuff[packet_idx++];
      
      pOutResult->BroadID = pBuff[packet_idx++];
      
      pOutResult->Cmd = pBuff[packet_idx++];
      
      for(i=0; i<8; ++i) pOutResult->Data[i] = pBuff[packet_idx++];
      break;
    }

    if(packet_len < total_len) total_len -= packet_len;
    else total_len = 0;
  }

  for(i=0; i<8; ++i)
  {
    //commented To debug galaxy
    if(com_name[i] != pCom_name[i]) return -1;               //Company name filter
  }

  if(pOutResult->ComID != 0xFFFF) return -1;
  if(pOutResult->BroadID != 0xFF) return -1;
  
  return 1;
}

/*
@brief : Debug for readio received data
*/
void dbg_nd_rf_rcv(void)
{
  if(rf_rx_buffer_g.idx_tail != rf_rx_buffer_g.idx_header)
  {
    //SEGGER_RTT_printf(0, "%02x %02x %02x %02x ", rf_rx_buffer_g.data[rf_rx_buffer_g.idx_tail][8], rf_rx_buffer_g.data[rf_rx_buffer_g.idx_tail][9], rf_rx_buffer_g.data[rf_rx_buffer_g.idx_tail][10], rf_rx_buffer_g.data[rf_rx_buffer_g.idx_tail][11]);
    //SEGGER_RTT_printf(0, "%02x %02x %02x %02x\r\n", rf_rx_buffer_g.data[rf_rx_buffer_g.idx_tail][12], rf_rx_buffer_g.data[rf_rx_buffer_g.idx_tail][13], rf_rx_buffer_g.data[rf_rx_buffer_g.idx_tail][14], rf_rx_buffer_g.data[rf_rx_buffer_g.idx_tail][15]);

    if(nd_rf_rcv_parser(rf_rx_buffer_g.data[rf_rx_buffer_g.idx_tail], &rfPacketParser_g) == 1)
    {
      //nrf_gpio_pin_clear(CHG_IND);
      
      RCVHandler_g(rfPacketParser_g.Cmd, rfPacketParser_g.Data);
    }
    
    ++rf_rx_buffer_g.idx_tail;
    if(rf_rx_buffer_g.idx_tail >= ND_RF_RXBUFFER_SIZE) rf_rx_buffer_g.idx_tail = 0;
  }
}

/*
* @brief : RF interrupt service routine
*/
void RADIO_IRQHandler(void)
{
  if(ND_RF_EVNT_DISABLED)
  {
    ND_RF_EVNT_DISABLED = 0;
  }

  if(ND_RF_EVNT_END)
  {
    ND_RF_EVNT_END = 0;
  }
}
//End of file
