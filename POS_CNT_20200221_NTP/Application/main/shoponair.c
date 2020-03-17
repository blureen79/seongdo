#include <string.h>
#include "nrf_gpio.h"
#include "nrf_delay.h"
#include "shoponair.h"
#include "custom_board.h"
#include "SEGGER_RTT.h"


void playErrorSound(bool DoReset)
{
  static uint8_t cntPlay = 0;
  
  if(DoReset == true)
  {
    cntPlay = 0;                            //Reset count
  }
  
  if(nrf_gpio_pin_read(DEF_AUDIO_BUSY) != 0) return;
  else
  {
    SoundPlay(72);
    ++cntPlay;
  }
}

void SoundVol(uint8_t value)
{
  uint8_t i;
  uint8_t delay_val = 1;
  uint8_t DeviceAddress = 0xDC;
  uint8_t RegisterAddress = 0xE1;
  uint8_t data = value;

  if(data > 15) data = 15;
  
  // data change
  switch(data)
  {
    case 0:
      data = 0x07;  // +17 dB
      break;
    case 4:
      data = 0x11;   // +7 dB
      break;
    case 8:
      data = 0x1B;  // -3 dB
      break;
    case 12:
      data = 0x25;  // -13 dB
      break;
    case 15:
      data = 0x2F;  // -23 dB
      break;
    default:
      data = 0x07;
      break;
  }
  
  SoundPU();

  nrf_gpio_pin_set(DEF_SOUND_SDO);
  nrf_gpio_pin_set(DEF_SOUND_SCK);

  nrf_delay_us(delay_val);
  nrf_gpio_pin_clear(DEF_SOUND_SDO);
  nrf_delay_us(delay_val);
  nrf_gpio_pin_clear(DEF_SOUND_SCK);
  nrf_delay_us(delay_val);

  for (i = 0; i < 8; ++i)
  {
    if ((DeviceAddress << i) & 0x80) nrf_gpio_pin_set(DEF_SOUND_SDO);
    else nrf_gpio_pin_clear(DEF_SOUND_SDO);
    nrf_delay_us(delay_val);
    nrf_gpio_pin_set(DEF_SOUND_SCK);
    nrf_delay_us(delay_val);
    nrf_gpio_pin_clear(DEF_SOUND_SCK);
    nrf_delay_us(delay_val);
  }
  
  nrf_gpio_pin_clear(DEF_SOUND_SDO);
  nrf_delay_us(delay_val);
  nrf_gpio_pin_set(DEF_SOUND_SCK);
  nrf_delay_us(delay_val);
  nrf_gpio_pin_clear(DEF_SOUND_SCK);
  nrf_delay_us(delay_val);
  nrf_gpio_pin_set(DEF_SOUND_SDO);
  nrf_delay_us(delay_val);
  
  for (i = 0; i < 8; ++i)
  {
    if ((RegisterAddress << i) & 0x80) nrf_gpio_pin_set(DEF_SOUND_SDO);
    else nrf_gpio_pin_clear(DEF_SOUND_SDO);
    nrf_delay_us(delay_val);
    nrf_gpio_pin_set(DEF_SOUND_SCK);
    nrf_delay_us(delay_val);
    nrf_gpio_pin_clear(DEF_SOUND_SCK);
    nrf_delay_us(delay_val);
  }

  nrf_gpio_pin_clear(DEF_SOUND_SDO);
  nrf_delay_us(delay_val);
  nrf_gpio_pin_set(DEF_SOUND_SCK);
  nrf_delay_us(delay_val);
  nrf_gpio_pin_clear(DEF_SOUND_SCK);
  nrf_delay_us(delay_val);
  nrf_gpio_pin_set(DEF_SOUND_SDO);
  nrf_delay_us(delay_val);
  
  for (i = 0; i < 8; ++i)
  {
    if ((data << i) & 0x80) nrf_gpio_pin_set(DEF_SOUND_SDO);
    else nrf_gpio_pin_clear(DEF_SOUND_SDO);
    nrf_delay_us(delay_val);
    nrf_gpio_pin_set(DEF_SOUND_SCK);
    nrf_delay_us(delay_val);
    nrf_gpio_pin_clear(DEF_SOUND_SCK);
    nrf_delay_us(delay_val);
  }

  nrf_gpio_pin_clear(DEF_SOUND_SDO);
  nrf_delay_us(delay_val);
  nrf_gpio_pin_set(DEF_SOUND_SCK);
  nrf_delay_us(delay_val);
  nrf_gpio_pin_clear(DEF_SOUND_SCK);
  nrf_delay_us(delay_val);
  nrf_gpio_pin_set(DEF_SOUND_SDO);
  nrf_delay_us(delay_val);
  
  nrf_delay_us(delay_val);
  nrf_gpio_pin_set(DEF_SOUND_SCK);
  nrf_delay_us(delay_val);
  nrf_gpio_pin_set(DEF_SOUND_SDO);
  nrf_delay_us(delay_val);
}

void SoundPU(void)
{
  //Play Monitor Option
  uint8_t i;
  uint32_t delay_val = DEF_SOUND_DELAY;
  uint8_t DeviceAddress = 0xDC;
  uint8_t RegisterAddress = 0xE3;
  uint8_t data = 0x05;  //Play Monitor Enable

  nrf_gpio_pin_set(DEF_SOUND_SDO);
  nrf_gpio_pin_set(DEF_SOUND_SCK);

  nrf_delay_us(delay_val);
  nrf_delay_us(delay_val);
  nrf_gpio_pin_clear(DEF_SOUND_SDO);
  nrf_delay_us(delay_val);
  nrf_gpio_pin_clear(DEF_SOUND_SCK);
  nrf_delay_us(delay_val);

  for (i = 0; i < 8; ++i)
  {
    if ((DeviceAddress << i) & 0x80) nrf_gpio_pin_set(DEF_SOUND_SDO);
    else nrf_gpio_pin_clear(DEF_SOUND_SDO);
    nrf_delay_us(delay_val);
    nrf_gpio_pin_set(DEF_SOUND_SCK);
    nrf_delay_us(delay_val);
    nrf_gpio_pin_clear(DEF_SOUND_SCK);
    nrf_delay_us(delay_val);
  }
  nrf_gpio_pin_clear(DEF_SOUND_SDO);
  nrf_delay_us(delay_val);
  nrf_gpio_pin_set(DEF_SOUND_SCK);
  nrf_delay_us(delay_val);
  nrf_gpio_pin_clear(DEF_SOUND_SCK);
  nrf_delay_us(delay_val);
  nrf_gpio_pin_set(DEF_SOUND_SDO);
  nrf_delay_us(delay_val);
  
  for (i = 0; i < 8; ++i)
  {
    if ((RegisterAddress << i) & 0x80) nrf_gpio_pin_set(DEF_SOUND_SDO);
    else nrf_gpio_pin_clear(DEF_SOUND_SDO);
    nrf_delay_us(delay_val);
    nrf_gpio_pin_set(DEF_SOUND_SCK);
    nrf_delay_us(delay_val);
    nrf_gpio_pin_clear(DEF_SOUND_SCK);
    nrf_delay_us(delay_val);
  }
  nrf_gpio_pin_clear(DEF_SOUND_SDO);
  nrf_delay_us(delay_val);
  nrf_gpio_pin_set(DEF_SOUND_SCK);
  nrf_delay_us(delay_val);
  nrf_gpio_pin_clear(DEF_SOUND_SCK);
  nrf_delay_us(delay_val);
  nrf_gpio_pin_set(DEF_SOUND_SDO);
  nrf_delay_us(delay_val);

  for (i = 0; i < 8; ++i)
  {
    if ((data << i) & 0x80) nrf_gpio_pin_set(DEF_SOUND_SDO);
    else nrf_gpio_pin_clear(DEF_SOUND_SDO);
    nrf_delay_us(delay_val);
    nrf_gpio_pin_set(DEF_SOUND_SCK);
    nrf_delay_us(delay_val);
    nrf_gpio_pin_clear(DEF_SOUND_SCK);
    nrf_delay_us(delay_val);
  }
  nrf_gpio_pin_clear(DEF_SOUND_SDO);
  nrf_delay_us(delay_val);
  nrf_gpio_pin_set(DEF_SOUND_SCK);
  nrf_delay_us(delay_val);
  nrf_gpio_pin_clear(DEF_SOUND_SCK);
  nrf_delay_us(delay_val);
  nrf_gpio_pin_set(DEF_SOUND_SDO);
  nrf_delay_us(delay_val);

  
  nrf_delay_us(delay_val);
  nrf_gpio_pin_set(DEF_SOUND_SCK);
  nrf_delay_us(delay_val);
  nrf_gpio_pin_set(DEF_SOUND_SDO);
  nrf_delay_us(delay_val);
  
  ///////////////////////////////////////////////////////////////
  DeviceAddress = 0xDC;
  RegisterAddress = 0xE2;
  data = 0x60;  // Play Signal Monitor Pin Mapping
  
  nrf_gpio_pin_set(DEF_SOUND_SDO);
  nrf_gpio_pin_set(DEF_SOUND_SCK);

  nrf_delay_us(delay_val);
  nrf_delay_us(delay_val);
  nrf_gpio_pin_clear(DEF_SOUND_SDO);
  nrf_delay_us(delay_val);
  nrf_gpio_pin_clear(DEF_SOUND_SCK);
  nrf_delay_us(delay_val);

  for (i = 0; i < 8; ++i)
  {
    if ((DeviceAddress << i) & 0x80) nrf_gpio_pin_set(DEF_SOUND_SDO);
    else nrf_gpio_pin_clear(DEF_SOUND_SDO);
    nrf_delay_us(delay_val);
    nrf_gpio_pin_set(DEF_SOUND_SCK);
    nrf_delay_us(delay_val);
    nrf_gpio_pin_clear(DEF_SOUND_SCK);
    nrf_delay_us(delay_val);
  }
  nrf_gpio_pin_clear(DEF_SOUND_SDO);
  nrf_delay_us(delay_val);
  nrf_gpio_pin_set(DEF_SOUND_SCK);
  nrf_delay_us(delay_val);
  nrf_gpio_pin_clear(DEF_SOUND_SCK);
  nrf_delay_us(delay_val);
  nrf_gpio_pin_set(DEF_SOUND_SDO);
  nrf_delay_us(delay_val);

  
  for (i = 0; i < 8; ++i)
  {
    if ((RegisterAddress << i) & 0x80) nrf_gpio_pin_set(DEF_SOUND_SDO);
    else nrf_gpio_pin_clear(DEF_SOUND_SDO);
    nrf_delay_us(delay_val);
    nrf_gpio_pin_set(DEF_SOUND_SCK);
    nrf_delay_us(delay_val);
    nrf_gpio_pin_clear(DEF_SOUND_SCK);
    nrf_delay_us(delay_val);
  }
  nrf_gpio_pin_clear(DEF_SOUND_SDO);
  nrf_delay_us(delay_val);
  nrf_gpio_pin_set(DEF_SOUND_SCK);
  nrf_delay_us(delay_val);
  nrf_gpio_pin_clear(DEF_SOUND_SCK);
  nrf_delay_us(delay_val);
  nrf_gpio_pin_set(DEF_SOUND_SDO);
  nrf_delay_us(delay_val);

  
  for (i = 0; i < 8; ++i)
  {
    if ((data << i) & 0x80) nrf_gpio_pin_set(DEF_SOUND_SDO);
    else nrf_gpio_pin_clear(DEF_SOUND_SDO);
    nrf_delay_us(delay_val);
    nrf_gpio_pin_set(DEF_SOUND_SCK);
    nrf_delay_us(delay_val);
    nrf_gpio_pin_clear(DEF_SOUND_SCK);
    nrf_delay_us(delay_val);
  }
  nrf_gpio_pin_clear(DEF_SOUND_SDO);
  nrf_delay_us(delay_val);
  nrf_gpio_pin_set(DEF_SOUND_SCK);
  nrf_delay_us(delay_val);
  nrf_gpio_pin_clear(DEF_SOUND_SCK);
  nrf_delay_us(delay_val);
  nrf_gpio_pin_set(DEF_SOUND_SDO);
  nrf_delay_us(delay_val);

  
  nrf_delay_us(delay_val);
  nrf_gpio_pin_set(DEF_SOUND_SCK);
  nrf_delay_us(delay_val);
  nrf_gpio_pin_set(DEF_SOUND_SDO);
  nrf_delay_us(delay_val);
  nrf_delay_us(delay_val);
  nrf_delay_us(delay_val);
}

void SoundPlay(uint16_t data)
{
  uint8_t i;
  uint32_t delay_val = DEF_SOUND_DELAY;
  uint8_t DeviceAddress = 0xDC;
  uint8_t RegisterAddress = 0xE4; // Phrase Number 2

  //SoundPU();

  nrf_gpio_pin_set(DEF_SOUND_SDO);
  nrf_gpio_pin_set(DEF_SOUND_SCK);

  nrf_delay_us(delay_val);
  nrf_delay_us(delay_val);
  nrf_gpio_pin_clear(DEF_SOUND_SDO);
  nrf_delay_us(delay_val);
  nrf_gpio_pin_clear(DEF_SOUND_SCK);
  nrf_delay_us(delay_val);

  for (i = 0; i < 8; ++i)
  {
    if ((DeviceAddress << i) & 0x80) nrf_gpio_pin_set(DEF_SOUND_SDO);
    else nrf_gpio_pin_clear(DEF_SOUND_SDO);
    nrf_delay_us(delay_val);
    nrf_gpio_pin_set(DEF_SOUND_SCK);
    nrf_delay_us(delay_val);
    nrf_gpio_pin_clear(DEF_SOUND_SCK);
    nrf_delay_us(delay_val);
  }
  nrf_gpio_pin_clear(DEF_SOUND_SDO);
  nrf_delay_us(delay_val);
  nrf_gpio_pin_set(DEF_SOUND_SCK);
  nrf_delay_us(delay_val);
  nrf_gpio_pin_clear(DEF_SOUND_SCK);
  nrf_delay_us(delay_val);
  nrf_gpio_pin_set(DEF_SOUND_SDO);
  nrf_delay_us(delay_val);
  
  for (i = 0; i < 8; ++i)
  {
    if ((RegisterAddress << i) & 0x80) nrf_gpio_pin_set(DEF_SOUND_SDO);
    else nrf_gpio_pin_clear(DEF_SOUND_SDO);
    nrf_delay_us(delay_val);
    nrf_gpio_pin_set(DEF_SOUND_SCK);
    nrf_delay_us(delay_val);
    nrf_gpio_pin_clear(DEF_SOUND_SCK);
    nrf_delay_us(delay_val);
  }
  nrf_gpio_pin_clear(DEF_SOUND_SDO);
  nrf_delay_us(delay_val);
  nrf_gpio_pin_set(DEF_SOUND_SCK);
  nrf_delay_us(delay_val);
  nrf_gpio_pin_clear(DEF_SOUND_SCK);
  nrf_delay_us(delay_val);
  nrf_gpio_pin_set(DEF_SOUND_SDO);
  nrf_delay_us(delay_val);
  
  for (i = 0; i < 8; ++i)
  {
    if ((0 << i) & 0x80) nrf_gpio_pin_set(DEF_SOUND_SDO);
    else nrf_gpio_pin_clear(DEF_SOUND_SDO);
    nrf_delay_us(delay_val);
    nrf_gpio_pin_set(DEF_SOUND_SCK);
    nrf_delay_us(delay_val);
    nrf_gpio_pin_clear(DEF_SOUND_SCK);
    nrf_delay_us(delay_val);
  }
  nrf_gpio_pin_clear(DEF_SOUND_SDO);
  nrf_delay_us(delay_val);
  nrf_gpio_pin_set(DEF_SOUND_SCK);
  nrf_delay_us(delay_val);
  nrf_gpio_pin_clear(DEF_SOUND_SCK);
  nrf_delay_us(delay_val);
  nrf_gpio_pin_set(DEF_SOUND_SDO);
  nrf_delay_us(delay_val);

  
  nrf_delay_us(delay_val);
  nrf_gpio_pin_set(DEF_SOUND_SCK);
  nrf_delay_us(delay_val);
  nrf_gpio_pin_set(DEF_SOUND_SDO);
  nrf_delay_us(delay_val);
  nrf_delay_us(delay_val);
  nrf_delay_us(delay_val);
  
  NRF_WDT->RR[0] = WDT_RR_RR_Reload;
  //SEGGER_RTT_printf(0, "\n 1. SoundPlayData = %02X %02X %02X",DeviceAddress,RegisterAddress,0);
  //////////////////////////////////////////////////////////////////////

  DeviceAddress = 0xDC;
  RegisterAddress = 0xE0; // Phrase Number 1

  nrf_gpio_pin_set(DEF_SOUND_SDO);
  nrf_gpio_pin_set(DEF_SOUND_SCK);

  nrf_delay_us(delay_val);
  nrf_delay_us(delay_val);
  nrf_gpio_pin_clear(DEF_SOUND_SDO);
  nrf_delay_us(delay_val);
  nrf_gpio_pin_clear(DEF_SOUND_SCK);
  nrf_delay_us(delay_val);

  for (i = 0; i < 8; ++i)
  {
    if ((DeviceAddress << i) & 0x80) nrf_gpio_pin_set(DEF_SOUND_SDO);
    else nrf_gpio_pin_clear(DEF_SOUND_SDO);
    nrf_delay_us(delay_val);
    nrf_gpio_pin_set(DEF_SOUND_SCK);
    nrf_delay_us(delay_val);
    nrf_gpio_pin_clear(DEF_SOUND_SCK);
    nrf_delay_us(delay_val);
  }
  nrf_gpio_pin_clear(DEF_SOUND_SDO);
  nrf_delay_us(delay_val);
  nrf_gpio_pin_set(DEF_SOUND_SCK);
  nrf_delay_us(delay_val);
  nrf_gpio_pin_clear(DEF_SOUND_SCK);
  nrf_delay_us(delay_val);
  nrf_gpio_pin_set(DEF_SOUND_SDO);
  nrf_delay_us(delay_val);

  
  for (i = 0; i < 8; ++i)
  {
    if ((RegisterAddress << i) & 0x80) nrf_gpio_pin_set(DEF_SOUND_SDO);
    else nrf_gpio_pin_clear(DEF_SOUND_SDO);
    nrf_delay_us(delay_val);
    nrf_gpio_pin_set(DEF_SOUND_SCK);
    nrf_delay_us(delay_val);
    nrf_gpio_pin_clear(DEF_SOUND_SCK);
    nrf_delay_us(delay_val);
  }
  nrf_gpio_pin_clear(DEF_SOUND_SDO);
  nrf_delay_us(delay_val);
  nrf_gpio_pin_set(DEF_SOUND_SCK);
  nrf_delay_us(delay_val);
  nrf_gpio_pin_clear(DEF_SOUND_SCK);
  nrf_delay_us(delay_val);
  nrf_gpio_pin_set(DEF_SOUND_SDO);
  nrf_delay_us(delay_val);
  
  for (i = 0; i < 8; ++i)
  {
    if ((data << i) & 0x80) nrf_gpio_pin_set(DEF_SOUND_SDO);
    else nrf_gpio_pin_clear(DEF_SOUND_SDO);
    nrf_delay_us(delay_val);
    nrf_gpio_pin_set(DEF_SOUND_SCK);
    nrf_delay_us(delay_val);
    nrf_gpio_pin_clear(DEF_SOUND_SCK);
    nrf_delay_us(delay_val);
  }
  nrf_gpio_pin_clear(DEF_SOUND_SDO);
  nrf_delay_us(delay_val);
  nrf_gpio_pin_set(DEF_SOUND_SCK);
  nrf_delay_us(delay_val);
  nrf_gpio_pin_clear(DEF_SOUND_SCK);
  nrf_delay_us(delay_val);
  nrf_gpio_pin_set(DEF_SOUND_SDO);
  nrf_delay_us(delay_val);

  nrf_delay_us(delay_val);
  nrf_gpio_pin_set(DEF_SOUND_SCK);
  nrf_delay_us(delay_val);
  nrf_gpio_pin_set(DEF_SOUND_SDO);
  nrf_delay_us(delay_val);
  nrf_delay_us(delay_val);
  nrf_delay_us(delay_val);
  
  nrf_delay_ms(16);
}

void totalHap(uint8_t* total) 
{
  uint8_t numOfDigits = 0, orgLen;
  uint8_t i = 0;
  uint32_t timeout = 0;

  numOfDigits = strlen((char*)total);
  orgLen = numOfDigits;
  SoundPU();

  for(; numOfDigits > 0; --numOfDigits)
  {
    i = orgLen - numOfDigits;
    SEGGER_RTT_printf(0, "\n%d , %02x ", numOfDigits, total[i]);

#if 1
    switch(numOfDigits)
    {
      case 7:
      if ((total[i + 1] == '0') && (total[i + 2] == '0'))
      {
        if (total[i] == '1')
        {
          SoundPlay(54); // "백만"
        }
        else if (total[i] == '2')
        {
          SoundPlay(55); // "이백만"
        }
        else if (total[i] == '3')
        {
          SoundPlay(56); // "삼백만"
        }
        else if (total[i] == '4')
        {
          SoundPlay(57); // "사백만"
        }
        else if (total[i] == '5')
        {
          SoundPlay(58); // "오백만"
        }
        else if (total[i] == '6')
        {
          SoundPlay(59); // "육백만"
        }
        else if (total[i] == '7')
        {
          SoundPlay(60); // "칠백만"
        }
        else if (total[i] == '8')
        {
          SoundPlay(61); // "팔백만"
        }
        else if (total[i] == '9')
        {
          SoundPlay(62); // "구백만"
        }
      }
      else 
      {
        if (total[i] == '1')
        {
          SoundPlay(18); // "백"
        }
        else if (total[i] == '2')
        {
          SoundPlay(19); // "이백"
        }
        else if (total[i] == '3')
        {
          SoundPlay(20); // "삼백"
        }
        else if (total[i] == '4')
        {
          SoundPlay(21); // "사백"
        }
        else if (total[i] == '5')
        {
          SoundPlay(22); // "오백"
        }
        else if (total[i] == '6')
        {
          SoundPlay(23); // "육백"
        }
        else if (total[i] == '7')
        {
          SoundPlay(24); // "칠백"
        }
        else if (total[i] == '8')
        {
          SoundPlay(25); // "팔백"
        }
        else if (total[i] == '9')
        {
          SoundPlay(26); // "구백"
        }
      }
      break;

      case 6:
      if (total[i + 1] == '0') 
      {
        if (total[i] == '1')
        {
          SoundPlay(45); // "십만"
        }
        else if (total[i] == '2')
        {
          SoundPlay(46); // "이십만"
        }
        else if (total[i] == '3')
        {
          SoundPlay(47); // "삼십만"
        }
        else if (total[i] == '4')
        {
          SoundPlay(48); // "사십만"
        }
        else if (total[i] == '5')
        {
          SoundPlay(49); // "오십만"
        }
        else if (total[i] == '6')
        {
          SoundPlay(50); // "육십만"
        }
        else if (total[i] == '7')
        {
          SoundPlay(51); // "칠십만"
        }
        else if (total[i] == '8')
        {
          SoundPlay(52); // "팔십만"
        }
        else if (total[i] == '9')
        {
          SoundPlay(53); // "구십만"
        }
      }
      else 
      {
        if (total[i] == '1')
        {
          SoundPlay(9); // "십"
        }
        else if (total[i] == '2')
        {
          SoundPlay(10); // "이십"
        }
        else if (total[i] == '3')
        {
          SoundPlay(11); // "삼십"
        }
        else if (total[i] == '4')
        {
          SoundPlay(12); // "사십"
        }
        else if (total[i] == '5')
        {
          SoundPlay(13); // "오십"
        }
        else if (total[i] == '6')
        {
          SoundPlay(14); // "육십"
        }
        else if (total[i] == '7')
        {
          SoundPlay(15); // "칠십"
        }
        else if (total[i] == '8')
        {
          SoundPlay(16); // "팔십"
        }
        else if (total[i] == '9')
        {
          SoundPlay(17); // "구십"
        }
      }
      break;

      case 5:
      if (total[i] == '1') // 예시) "110,000" 십일만원으로 출력해야하는 조건
      {
        SoundPlay(36); // "만" Sound IC 교체 후 이 값을 '78' 으로 교체 할것
      }
      else if (total[i] == '2')
      {
        SoundPlay(37); // "이만"
      }
      else if (total[i] == '3')
      {
        SoundPlay(38); // "삼만"
      }
      else if (total[i] == '4')
      {
        SoundPlay(39); // "사만"
      }
      else if (total[i] == '5')
      {
        SoundPlay(40); // "오만"
      }
      else if (total[i] == '6')
      {
        SoundPlay(41); // "육만"
      }
      else if (total[i] == '7')
      {
        SoundPlay(42); // "칠만"
      }
      else if (total[i] == '8')
      {
        SoundPlay(43); // "팔만"
      }
      else if (total[i] == '9')
      {
        SoundPlay(44); // "구만"
      }
      break;

      case 4:
      if (total[i] == '1')
      {
        SoundPlay(27); // "천"
      }
      else if (total[i] == '2')
      {
        SoundPlay(28); // "이천"
      }
      else if (total[i] == '3')
      {
        SoundPlay(29); // "삼천"
      }
      else if (total[i] == '4')
      {
        SoundPlay(30); // "사천"
      }
      else if (total[i] == '5')
      {
        SoundPlay(31); // "오천"
      }
      else if (total[i] == '6')
      {
        SoundPlay(32); // "육천"
      }
      else if (total[i] == '7')
      {
        SoundPlay(33); // "칠천"
      }
      else if (total[i] == '8')
      {
        SoundPlay(34); // "팔천"
      }
      else if (total[i] == '9')
      {
        SoundPlay(35); // "구천"
      }
      break;

      case 3:
      if (total[i] == '1')
      {
        SoundPlay(18); // "백"
      }
      else if (total[i] == '2')
      {
        SoundPlay(19); // "이백"
      }
      else if (total[i] == '3')
      {
        SoundPlay(20); // "삼백"
      }
      else if (total[i] == '4')
      {
        SoundPlay(21); // "사백"
      }
      else if (total[i] == '5')
      {
        SoundPlay(22); // "오백"
      }
      else if (total[i] == '6')
      {
        SoundPlay(23); // "육백"
      }
      else if (total[i] == '7')
      {
        SoundPlay(24); // "칠백"
      }
      else if (total[i] == '8')
      {
        SoundPlay(25); // "팔백"
      }
      else if (total[i] == '9')
      {
        SoundPlay(26); // "구백"
      }
      break;

      case 2:
      if (total[i] == '1')
      {
        SoundPlay(9); // "십"
      }
      else if (total[i] == '2')
      {
        SoundPlay(10); // "이십"
      }
      else if (total[i] == '3')
      {
        SoundPlay(11); // "삼십"
      }
      else if (total[i] == '4')
      {
        SoundPlay(12); // "사십"
      }
      else if (total[i] == '5')
      {
        SoundPlay(13); // "오십"
      }
      else if (total[i] == '6')
      {
        SoundPlay(14); // "육십"
      }
      else if (total[i] == '7')
      {
        SoundPlay(15); // "칠십"
      }
      else if (total[i] == '8')
      {
        SoundPlay(16); // "팔십"
      }
      else if (total[i] == '9')
      {
        SoundPlay(17); // "구십"
      }
      break;

      case 1:
      if (total[i] == '1')
      {
        SoundPlay(0); //"일원"
      }
      else if (total[i] == '2')
      {
        SoundPlay(1); //"이원"
      }
      else if (total[i] == '3')
      {
        SoundPlay(2); //"삼원"
      }
      else if (total[i] == '4')
      {
        SoundPlay(3); //"사원"
      }
      else if (total[i] == '5')
      {
        SoundPlay(4); //"오원"
      }
      else if (total[i] == '6')
      {
        SoundPlay(5); //"육원"
      }
      else if (total[i] == '7')
      {
        SoundPlay(6); //"칠원"
      }
      else if (total[i] == '8')
      {
        SoundPlay(7); //"팔원"
      }
      else if (total[i] == '9')
      {
        SoundPlay(8); //"구원"
      }
      else if (total[i] == '0')
      {
        SoundPlay(63); //"원"
      }
      break;

      case 0:
      break;

      default:
      break;
    }
#else
    // j == 8 인경우 천만원 단위가 입력된 경우 미처리 하고 부즈음 삐~ 발생후 종료
    if (numOfDigits >= 8) 
    {
      return;
    }
    //백만 단위
    else if (numOfDigits == 7)
    {
      if ((total[i + 1] == '0') && (total[i + 2] == '0'))
      {
        if (total[i] == '1')
        {
          SoundPlay(54); // "백만"
        }
        else if (total[i] == '2')
        {
          SoundPlay(55); // "이백만"
        }
        else if (total[i] == '3')
        {
          SoundPlay(56); // "삼백만"
        }
        else if (total[i] == '4')
        {
          SoundPlay(57); // "사백만"
        }
        else if (total[i] == '5')
        {
          SoundPlay(58); // "오백만"
        }
        else if (total[i] == '6')
        {
          SoundPlay(59); // "육백만"
        }
        else if (total[i] == '7')
        {
          SoundPlay(60); // "칠백만"
        }
        else if (total[i] == '8')
        {
          SoundPlay(61); // "팔백만"
        }
        else if (total[i] == '9')
        {
          SoundPlay(62); // "구백만"
        }
      }
      else 
      {
        if (total[i] == '1')
        {
          SoundPlay(18); // "백"
        }
        else if (total[i] == '2')
        {
          SoundPlay(19); // "이백"
        }
        else if (total[i] == '3')
        {
          SoundPlay(20); // "삼백"
        }
        else if (total[i] == '4')
        {
          SoundPlay(21); // "사백"
        }
        else if (total[i] == '5')
        {
          SoundPlay(22); // "오백"
        }
        else if (total[i] == '6')
        {
          SoundPlay(23); // "육백"
        }
        else if (total[i] == '7')
        {
          SoundPlay(24); // "칠백"
        }
        else if (total[i] == '8')
        {
          SoundPlay(25); // "팔백"
        }
        else if (total[i] == '9')
        {
          SoundPlay(26); // "구백"
        }
      }
    }
    //십만 단위
    else if (numOfDigits == 6) 
    {
      if (total[i + 1] == '0') 
      {
        if (total[i] == '1')
        {
          SoundPlay(45); // "십만"
        }
        else if (total[i] == '2')
        {
          SoundPlay(46); // "이십만"
        }
        else if (total[i] == '3')
        {
          SoundPlay(47); // "삼십만"
        }
        else if (total[i] == '4')
        {
          SoundPlay(48); // "사십만"
        }
        else if (total[i] == '5')
        {
          SoundPlay(49); // "오십만"
        }
        else if (total[i] == '6')
        {
          SoundPlay(50); // "육십만"
        }
        else if (total[i] == '7')
        {
          SoundPlay(51); // "칠십만"
        }
        else if (total[i] == '8')
        {
          SoundPlay(52); // "팔십만"
        }
        else if (total[i] == '9')
        {
          SoundPlay(53); // "구십만"
        }
      }
      else 
      {
        if (total[i] == '1')
        {
          SoundPlay(9); // "십"
        }
        else if (total[i] == '2')
        {
          SoundPlay(10); // "이십"
        }
        else if (total[i] == '3')
        {
          SoundPlay(11); // "삼십"
        }
        else if (total[i] == '4')
        {
          SoundPlay(12); // "사십"
        }
        else if (total[i] == '5')
        {
          SoundPlay(13); // "오십"
        }
        else if (total[i] == '6')
        {
          SoundPlay(14); // "육십"
        }
        else if (total[i] == '7')
        {
          SoundPlay(15); // "칠십"
        }
        else if (total[i] == '8')
        {
          SoundPlay(16); // "팔십"
        }
        else if (total[i] == '9')
        {
          SoundPlay(17); // "구십"
        }
      }
    }
    //만 단위
    else if (numOfDigits == 5) 
    {
      if (total[i] == '1')
      {
        SoundPlay(36); // "만"
      }
      else if (total[i] == '2')
      {
        SoundPlay(37); // "이만"
      }
      else if (total[i] == '3')
      {
        SoundPlay(38); // "삼만"
      }
      else if (total[i] == '4')
      {
        SoundPlay(39); // "사만"
      }
      else if (total[i] == '5')
      {
        SoundPlay(40); // "오만"
      }
      else if (total[i] == '6')
      {
        SoundPlay(41); // "육만"
      }
      else if (total[i] == '7')
      {
        SoundPlay(42); // "칠만"
      }
      else if (total[i] == '8')
      {
        SoundPlay(43); // "팔만"
      }
      else if (total[i] == '9')
      {
        SoundPlay(44); // "구만"
      }
    }
    //천 단위
    else if (numOfDigits == 4) 
    {
      if (total[i] == '1')
      {
        SoundPlay(27); // "천"
      }
      else if (total[i] == '2')
      {
        SoundPlay(28); // "이천"
      }
      else if (total[i] == '3')
      {
        SoundPlay(29); // "삼천"
      }
      else if (total[i] == '4')
      {
        SoundPlay(30); // "사천"
      }
      else if (total[i] == '5')
      {
        SoundPlay(31); // "오천"
      }
      else if (total[i] == '6')
      {
        SoundPlay(32); // "육천"
      }
      else if (total[i] == '7')
      {
        SoundPlay(33); // "칠천"
      }
      else if (total[i] == '8')
      {
        SoundPlay(34); // "팔천"
      }
      else if (total[i] == '9')
      {
        SoundPlay(35); // "구천"
      }
    }
    //백 단위
    else if (numOfDigits == 3) 
    {
      if (total[i] == '1')
      {
        SoundPlay(18); // "백"
      }
      else if (total[i] == '2')
      {
        SoundPlay(19); // "이백"
      }
      else if (total[i] == '3')
      {
        SoundPlay(20); // "삼백"
      }
      else if (total[i] == '4')
      {
        SoundPlay(21); // "사백"
      }
      else if (total[i] == '5')
      {
        SoundPlay(22); // "오백"
      }
      else if (total[i] == '6')
      {
        SoundPlay(23); // "육백"
      }
      else if (total[i] == '7')
      {
        SoundPlay(24); // "칠백"
      }
      else if (total[i] == '8')
      {
        SoundPlay(25); // "팔백"
      }
      else if (total[i] == '9')
      {
        SoundPlay(26); // "구백"
      }
    }
    //십 단위
    else if (numOfDigits == 2) 
    {
      if (total[i] == '1')
      {
        SoundPlay(9); // "십"
      }
      else if (total[i] == '2')
      {
        SoundPlay(10); // "이십"
      }
      else if (total[i] == '3')
      {
        SoundPlay(11); // "삼십"
      }
      else if (total[i] == '4')
      {
        SoundPlay(12); // "사십"
      }
      else if (total[i] == '5')
      {
        SoundPlay(13); // "오십"
      }
      else if (total[i] == '6')
      {
        SoundPlay(14); // "육십"
      }
      else if (total[i] == '7')
      {
        SoundPlay(15); // "칠십"
      }
      else if (total[i] == '8')
      {
        SoundPlay(16); // "팔십"
      }
      else if (total[i] == '9')
      {
        SoundPlay(17); // "구십"
      }
    }
    //일 단위
    else if (numOfDigits == 1) 
    {
      if (total[i] == '1')
      {
        SoundPlay(0); //"일원"
      }
      else if (total[i] == '2')
      {
        SoundPlay(1); //"이원"
      }
      else if (total[i] == '3')
      {
        SoundPlay(2); //"삼원"
      }
      else if (total[i] == '4')
      {
        SoundPlay(3); //"사원"
      }
      else if (total[i] == '5')
      {
        SoundPlay(4); //"오원"
      }
      else if (total[i] == '6')
      {
        SoundPlay(5); //"육원"
      }
      else if (total[i] == '7')
      {
        SoundPlay(6); //"칠원"
      }
      else if (total[i] == '8')
      {
        SoundPlay(7); //"팔원"
      }
      else if (total[i] == '9')
      {
        SoundPlay(8); //"구원"
      }
      else if (total[i] == '0')
      {
        SoundPlay(63); //"원"
      }
    }
    else
    {
      break;
    }
#endif

    //while (nrf_gpio_pin_read(DEF_AUDIO_BUSY) == 1) // Sound Busy Check
    for(timeout = 0xFFFF; timeout>0; --timeout)
    {
      if(nrf_gpio_pin_read(DEF_AUDIO_BUSY) == 0) break;    

      //PRINT "." , INTEVAL 50 ms
      //Serial.print(".");
      SEGGER_RTT_printf(0, ".");
      nrf_delay_ms(10);
      
      NRF_WDT->RR[0] = WDT_RR_RR_Reload;
    }
    //numOfDigits--;
    //i++;
  }

  SoundPlay(77); //"결제되었습니다"
  //SoundPlay(71); // CASHREG.WAV
  
  //while (nrf_gpio_pin_read(DEF_AUDIO_BUSY) == 1) // Sound Busy Check
  for(timeout = 0xFFFF; timeout>0; --timeout)
  {
    if(nrf_gpio_pin_read(DEF_AUDIO_BUSY) == 0) break;

    //PRINT "." , INTEVAL 50 ms
    //Serial.print(".");
    nrf_delay_ms(10);
    
    NRF_WDT->RR[0] = WDT_RR_RR_Reload;
  }  
}
