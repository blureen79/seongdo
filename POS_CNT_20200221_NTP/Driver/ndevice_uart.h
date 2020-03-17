/*
Author : Jaeyoung, LEE(NDevice.com)
Date : 06.21.2017

Converted for nRF51822, based on nRF52832
*/
#ifndef __H_NDEVICE_UART
#define __H_NDEVICE_UART

#define COMM_SOF                0x02

extern void bytesToHexString(uint8_t *bytes, uint8_t *outVal, int buflen);
extern void uart_setBaudrate(uint32_t value);
extern void uart_putstring(uint8_t * str);
extern void uart_init(uint8_t txd_pin_number, uint8_t rxd_pin_number);
extern bool uart_get_RXD(uint8_t* data);
extern bool uart_put(uint8_t data);
#endif
