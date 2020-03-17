#include "nrf.h"
#include "nrf_gpio.h"
#include "ndevice_uart.h"

#define QUEUE_SIZE        200  

static uint8_t uart_queue_g[QUEUE_SIZE];
static uint16_t idx_uart_hdr_g, idx_uart_tail_g;

static char byteMap[] = { '0', '1', '2', '3', '4', '5', '6', '7', '8', '9', 'a', 'b', 'c', 'd', 'e', 'f' };

#define SOA_BUADRATE_57600  (0x00F0F000UL) /* 57586bps */

void uart_setBaudrate(uint32_t value)
{
  switch(value)
  {
  case 9600:
    NRF_UART0->BAUDRATE      = (UART_BAUDRATE_BAUDRATE_Baud9600 << UART_BAUDRATE_BAUDRATE_Pos);
    break;
    
  case 19200:
    NRF_UART0->BAUDRATE      = (UART_BAUDRATE_BAUDRATE_Baud19200 << UART_BAUDRATE_BAUDRATE_Pos);
    break;
    
  case 38400:
    NRF_UART0->BAUDRATE      = (UART_BAUDRATE_BAUDRATE_Baud38400 << UART_BAUDRATE_BAUDRATE_Pos);
    break;
#ifndef SPECIAL_BAUD_RATE_57600    
  case 57600:
    //NRF_UART0->BAUDRATE      = (UART_BAUDRATE_BAUDRATE_Baud57600 << UART_BAUDRATE_BAUDRATE_Pos); //17.2us
    NRF_UART0->BAUDRATE      = (SOA_BUADRATE_57600 << UART_BAUDRATE_BAUDRATE_Pos); //17.2us
    break;
#endif    
  case 115200:
    NRF_UART0->BAUDRATE      = (UART_BAUDRATE_BAUDRATE_Baud115200 << UART_BAUDRATE_BAUDRATE_Pos);
    break;
    
  default:
    NRF_UART0->BAUDRATE      = (UART_BAUDRATE_BAUDRATE_Baud9600 << UART_BAUDRATE_BAUDRATE_Pos);
    break;
  }
}

void uart_init(uint8_t txd_pin_number, uint8_t rxd_pin_number)
{
/** @snippet [Configure UART RX and TX pin] */
    nrf_gpio_cfg_output(txd_pin_number);
    nrf_gpio_cfg_input(rxd_pin_number, NRF_GPIO_PIN_NOPULL);

    NRF_UART0->PSELTXD = txd_pin_number;
    NRF_UART0->PSELRXD = rxd_pin_number;
/** @snippet [Configure UART RX and TX pin] */

    NRF_UART0->BAUDRATE      = (UART_BAUDRATE_BAUDRATE_Baud9600 << UART_BAUDRATE_BAUDRATE_Pos);
    NRF_UART0->ENABLE        = (UART_ENABLE_ENABLE_Enabled << UART_ENABLE_ENABLE_Pos);
    NRF_UART0->TASKS_STARTTX = 1;
    NRF_UART0->TASKS_STARTRX = 1;
    NRF_UART0->EVENTS_RXDRDY = 0;
    NRF_UART0->EVENTS_TXDRDY = 0;

    //added by NDevice
    NRF_UART0->INTENSET = UART_INTENSET_RXDRDY_Enabled << UART_INTENSET_RXDRDY_Pos;	
    NVIC_SetPriority(UART0_IRQn, 2);
    NVIC_EnableIRQ(UART0_IRQn);
    //End of addition	

    idx_uart_hdr_g = 0;
    idx_uart_tail_g = 0;
}

/*
* brief : polling RCD
*/
bool uart_get_RXD(uint8_t* data)
{
  if(idx_uart_hdr_g == idx_uart_tail_g) return false;
  
  *data = uart_queue_g[idx_uart_tail_g++];
  if(idx_uart_tail_g >= QUEUE_SIZE) idx_uart_tail_g = 0;
  
  return true;
}

/*
* @brief : Send UART data
*/
bool uart_put(uint8_t data)
{
  bool ret = false;

#if 1
  NRF_UART0->TXD = (uint8_t)data;

  while (NRF_UART0->EVENTS_TXDRDY != 1)
  {
      // Wait for TXD data to be sent.
  }

  NRF_UART0->EVENTS_TXDRDY = 0;
#else
  if(NRF_UART0->EVENTS_TXDRDY == 1)
  {
      NRF_UART0->TXD = (uint8_t)data;
      ret = true;
  }
  else ret = false;
#endif


  return ret;  
}

/*
* @brief : Send UART data
*/
void uart_putstring(uint8_t * str)
{
    uint_fast8_t i  = 0;
    uint8_t      ch = str[i++];

    while (ch != '\0')
    {
        uart_put(ch);
        ch = str[i++];
    }
}

void queue_uart(uint8_t *pBuff, uint8_t len)
{
  uint8_t i;
  
  for(i=0; i<len; ++i)
  {
    uart_queue_g[idx_uart_hdr_g++] = pBuff[i];
    if(idx_uart_hdr_g >= QUEUE_SIZE) idx_uart_hdr_g = 0;
  }
}

void send_queued_uart(void)
{
  if(idx_uart_hdr_g != idx_uart_tail_g)
  //while(idx_uart_hdr_g != idx_uart_tail_g)
  {
    uart_put(uart_queue_g[idx_uart_tail_g++]);
    if(idx_uart_tail_g >= QUEUE_SIZE) idx_uart_tail_g = 0;
  }
}


/* Utility function to convert nibbles (4 bit values) into a hex character representation */
static char nibbleToChar(uint8_t nibble)
{
	if(nibble < 16) return byteMap[nibble];
	return '*';
}

/* Convert a buffer of binary values into a hex string representation */
void bytesToHexString(uint8_t *bytes, uint8_t *outVal, int buflen)
{
	int i;

	for(i=0; i<buflen; i++) {
		outVal[i*2] = nibbleToChar(bytes[i] >> 4);
		outVal[i*2+1] = nibbleToChar(bytes[i] & 0x0f);
	}
    
    outVal[i*2] = '\0';
}

/*
* @brief : ISR for uart
*/
void UART0_IRQHandler(void)
{
  NRF_UART0->EVENTS_RXDRDY = 0;

  uart_queue_g[idx_uart_hdr_g++] = (uint8_t)NRF_UART0->RXD;
  if(idx_uart_hdr_g >= QUEUE_SIZE) idx_uart_hdr_g = 0;
}

