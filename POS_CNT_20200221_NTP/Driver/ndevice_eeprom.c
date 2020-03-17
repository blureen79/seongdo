#include "ndevice_eeprom.h"
#include "ndevice_spi.h"

static uint8_t spi_tx_buff_g[EEP_BUFF_SIZE + 10];

/*
* @brief : eeprim busy status
*/
uint8_t IsEEPBusy(void)
{
  return IsSPIBusy();
}

/*
* @brief : Read array data
*/
void eep_read_array(uint16_t addr, uint8_t *pOutBuff, uint32_t len, fnSPIReceiveHandler handler)
{
  uint8_t sending_size;
  
  sending_size = 0;
  spi_tx_buff_g[sending_size++] = EEP_CMD_READARRAY;
  //send_data[sending_size++] = EEP_CMD_DEVICEID;               //Device ID tested
  spi_tx_buff_g[sending_size++] = (uint8_t)(addr >> 8);
  spi_tx_buff_g[sending_size++] = (uint8_t)(addr);

  spim_repeateddata(spi_tx_buff_g, sending_size, pOutBuff, len + sending_size, handler);
}

/*
* @brief : erase eeprom
*/
void eep_erase(uint8_t block_size)
{
  uint8_t sending_size;

  sending_size = 0;
  switch(block_size)
  {
  case 4:
    spi_tx_buff_g[sending_size++] = EEP_CMD_ERASEBLK_4K;
    break;
  case 32:
    spi_tx_buff_g[sending_size++] = EEP_CMD_ERASEBLK_32K;
    break;
  case 64:
    spi_tx_buff_g[sending_size++] = EEP_CMD_ERASEBLK_64K;
    break;    
  case 100:
    spi_tx_buff_g[sending_size++] = EEP_CMD_ERASECHIP;
    break;
  default:
    spi_tx_buff_g[sending_size++] = EEP_CMD_ERASEBLK_4K;
    break;
  }

  spim_senddata(spi_tx_buff_g, sending_size);
}

/*
* @brief : read a byte
*/
void eep_read_byte(uint16_t addr, fnSPIReceiveHandler handler)
{
}

/*
* @brief : read eeprom status
*/
void eep_read_status(uint8_t *pOutBuff, uint32_t len, fnSPIReceiveHandler handler)
{
  uint8_t sending_size;

  sending_size = 0;
  spi_tx_buff_g[sending_size++] = EEP_CMD_STATUS1;
  spi_tx_buff_g[sending_size++] = 0x00;             //dummy to read
  spi_tx_buff_g[sending_size++] = 0x00;             //dummy to read
  spi_tx_buff_g[sending_size++] = 0x00;             //dummy to read

  spim_repeateddata(spi_tx_buff_g, sending_size, pOutBuff, len + sending_size, handler);
}

/*
* @brief : write eeprom status
*/
void eep_write_status(uint8_t status)
{
  uint8_t sending_size;

  sending_size = 0;
  spi_tx_buff_g[sending_size++] = EEP_CMD_WRSR;
  spi_tx_buff_g[sending_size++] = status;             //dummy to read

  spim_senddata(spi_tx_buff_g, 2);
}

/*
* @brief : write array
*/
void eep_write_array(uint16_t addr, uint8_t* pInBuff, uint32_t len)
{
  uint32_t sending_size, i;
  
  sending_size = 0;
  spi_tx_buff_g[sending_size++] = EEP_CMD_WRITE;
  spi_tx_buff_g[sending_size++] = (uint8_t)(addr >> 8);
  spi_tx_buff_g[sending_size++] = (uint8_t)(addr);

  for(i=0; i<len; ++i) spi_tx_buff_g[sending_size++] = pInBuff[i];

  spim_senddata(spi_tx_buff_g, sending_size);  
}

/*
* @brief : eep enable/disable
setEnable : TRUE->write enable, FALSE->write disable
*/
void eep_write_enable(uint8_t setEnable)
{
  uint8_t sending_size;

  sending_size = 0;
  if(setEnable == true) spi_tx_buff_g[sending_size++] = EEP_CMD_WRITE_EN;
  else spi_tx_buff_g[sending_size++] = EEP_CMD_WRITE_DIS;

  spim_senddata(spi_tx_buff_g, sending_size);
}
