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
          SoundPlay(54); // "�鸸"
        }
        else if (total[i] == '2')
        {
          SoundPlay(55); // "�̹鸸"
        }
        else if (total[i] == '3')
        {
          SoundPlay(56); // "��鸸"
        }
        else if (total[i] == '4')
        {
          SoundPlay(57); // "��鸸"
        }
        else if (total[i] == '5')
        {
          SoundPlay(58); // "���鸸"
        }
        else if (total[i] == '6')
        {
          SoundPlay(59); // "���鸸"
        }
        else if (total[i] == '7')
        {
          SoundPlay(60); // "ĥ�鸸"
        }
        else if (total[i] == '8')
        {
          SoundPlay(61); // "�ȹ鸸"
        }
        else if (total[i] == '9')
        {
          SoundPlay(62); // "���鸸"
        }
      }
      else 
      {
        if (total[i] == '1')
        {
          SoundPlay(18); // "��"
        }
        else if (total[i] == '2')
        {
          SoundPlay(19); // "�̹�"
        }
        else if (total[i] == '3')
        {
          SoundPlay(20); // "���"
        }
        else if (total[i] == '4')
        {
          SoundPlay(21); // "���"
        }
        else if (total[i] == '5')
        {
          SoundPlay(22); // "����"
        }
        else if (total[i] == '6')
        {
          SoundPlay(23); // "����"
        }
        else if (total[i] == '7')
        {
          SoundPlay(24); // "ĥ��"
        }
        else if (total[i] == '8')
        {
          SoundPlay(25); // "�ȹ�"
        }
        else if (total[i] == '9')
        {
          SoundPlay(26); // "����"
        }
      }
      break;

      case 6:
      if (total[i + 1] == '0') 
      {
        if (total[i] == '1')
        {
          SoundPlay(45); // "�ʸ�"
        }
        else if (total[i] == '2')
        {
          SoundPlay(46); // "�̽ʸ�"
        }
        else if (total[i] == '3')
        {
          SoundPlay(47); // "��ʸ�"
        }
        else if (total[i] == '4')
        {
          SoundPlay(48); // "��ʸ�"
        }
        else if (total[i] == '5')
        {
          SoundPlay(49); // "���ʸ�"
        }
        else if (total[i] == '6')
        {
          SoundPlay(50); // "���ʸ�"
        }
        else if (total[i] == '7')
        {
          SoundPlay(51); // "ĥ�ʸ�"
        }
        else if (total[i] == '8')
        {
          SoundPlay(52); // "�Ƚʸ�"
        }
        else if (total[i] == '9')
        {
          SoundPlay(53); // "���ʸ�"
        }
      }
      else 
      {
        if (total[i] == '1')
        {
          SoundPlay(9); // "��"
        }
        else if (total[i] == '2')
        {
          SoundPlay(10); // "�̽�"
        }
        else if (total[i] == '3')
        {
          SoundPlay(11); // "���"
        }
        else if (total[i] == '4')
        {
          SoundPlay(12); // "���"
        }
        else if (total[i] == '5')
        {
          SoundPlay(13); // "����"
        }
        else if (total[i] == '6')
        {
          SoundPlay(14); // "����"
        }
        else if (total[i] == '7')
        {
          SoundPlay(15); // "ĥ��"
        }
        else if (total[i] == '8')
        {
          SoundPlay(16); // "�Ƚ�"
        }
        else if (total[i] == '9')
        {
          SoundPlay(17); // "����"
        }
      }
      break;

      case 5:
      if (total[i] == '1') // ����) "110,000" ���ϸ������� ����ؾ��ϴ� ����
      {
        SoundPlay(36); // "��" Sound IC ��ü �� �� ���� '78' ���� ��ü �Ұ�
      }
      else if (total[i] == '2')
      {
        SoundPlay(37); // "�̸�"
      }
      else if (total[i] == '3')
      {
        SoundPlay(38); // "�︸"
      }
      else if (total[i] == '4')
      {
        SoundPlay(39); // "�縸"
      }
      else if (total[i] == '5')
      {
        SoundPlay(40); // "����"
      }
      else if (total[i] == '6')
      {
        SoundPlay(41); // "����"
      }
      else if (total[i] == '7')
      {
        SoundPlay(42); // "ĥ��"
      }
      else if (total[i] == '8')
      {
        SoundPlay(43); // "�ȸ�"
      }
      else if (total[i] == '9')
      {
        SoundPlay(44); // "����"
      }
      break;

      case 4:
      if (total[i] == '1')
      {
        SoundPlay(27); // "õ"
      }
      else if (total[i] == '2')
      {
        SoundPlay(28); // "��õ"
      }
      else if (total[i] == '3')
      {
        SoundPlay(29); // "��õ"
      }
      else if (total[i] == '4')
      {
        SoundPlay(30); // "��õ"
      }
      else if (total[i] == '5')
      {
        SoundPlay(31); // "��õ"
      }
      else if (total[i] == '6')
      {
        SoundPlay(32); // "��õ"
      }
      else if (total[i] == '7')
      {
        SoundPlay(33); // "ĥõ"
      }
      else if (total[i] == '8')
      {
        SoundPlay(34); // "��õ"
      }
      else if (total[i] == '9')
      {
        SoundPlay(35); // "��õ"
      }
      break;

      case 3:
      if (total[i] == '1')
      {
        SoundPlay(18); // "��"
      }
      else if (total[i] == '2')
      {
        SoundPlay(19); // "�̹�"
      }
      else if (total[i] == '3')
      {
        SoundPlay(20); // "���"
      }
      else if (total[i] == '4')
      {
        SoundPlay(21); // "���"
      }
      else if (total[i] == '5')
      {
        SoundPlay(22); // "����"
      }
      else if (total[i] == '6')
      {
        SoundPlay(23); // "����"
      }
      else if (total[i] == '7')
      {
        SoundPlay(24); // "ĥ��"
      }
      else if (total[i] == '8')
      {
        SoundPlay(25); // "�ȹ�"
      }
      else if (total[i] == '9')
      {
        SoundPlay(26); // "����"
      }
      break;

      case 2:
      if (total[i] == '1')
      {
        SoundPlay(9); // "��"
      }
      else if (total[i] == '2')
      {
        SoundPlay(10); // "�̽�"
      }
      else if (total[i] == '3')
      {
        SoundPlay(11); // "���"
      }
      else if (total[i] == '4')
      {
        SoundPlay(12); // "���"
      }
      else if (total[i] == '5')
      {
        SoundPlay(13); // "����"
      }
      else if (total[i] == '6')
      {
        SoundPlay(14); // "����"
      }
      else if (total[i] == '7')
      {
        SoundPlay(15); // "ĥ��"
      }
      else if (total[i] == '8')
      {
        SoundPlay(16); // "�Ƚ�"
      }
      else if (total[i] == '9')
      {
        SoundPlay(17); // "����"
      }
      break;

      case 1:
      if (total[i] == '1')
      {
        SoundPlay(0); //"�Ͽ�"
      }
      else if (total[i] == '2')
      {
        SoundPlay(1); //"�̿�"
      }
      else if (total[i] == '3')
      {
        SoundPlay(2); //"���"
      }
      else if (total[i] == '4')
      {
        SoundPlay(3); //"���"
      }
      else if (total[i] == '5')
      {
        SoundPlay(4); //"����"
      }
      else if (total[i] == '6')
      {
        SoundPlay(5); //"����"
      }
      else if (total[i] == '7')
      {
        SoundPlay(6); //"ĥ��"
      }
      else if (total[i] == '8')
      {
        SoundPlay(7); //"�ȿ�"
      }
      else if (total[i] == '9')
      {
        SoundPlay(8); //"����"
      }
      else if (total[i] == '0')
      {
        SoundPlay(63); //"��"
      }
      break;

      case 0:
      break;

      default:
      break;
    }
#else
    // j == 8 �ΰ�� õ���� ������ �Էµ� ��� ��ó�� �ϰ� ������ ��~ �߻��� ����
    if (numOfDigits >= 8) 
    {
      return;
    }
    //�鸸 ����
    else if (numOfDigits == 7)
    {
      if ((total[i + 1] == '0') && (total[i + 2] == '0'))
      {
        if (total[i] == '1')
        {
          SoundPlay(54); // "�鸸"
        }
        else if (total[i] == '2')
        {
          SoundPlay(55); // "�̹鸸"
        }
        else if (total[i] == '3')
        {
          SoundPlay(56); // "��鸸"
        }
        else if (total[i] == '4')
        {
          SoundPlay(57); // "��鸸"
        }
        else if (total[i] == '5')
        {
          SoundPlay(58); // "���鸸"
        }
        else if (total[i] == '6')
        {
          SoundPlay(59); // "���鸸"
        }
        else if (total[i] == '7')
        {
          SoundPlay(60); // "ĥ�鸸"
        }
        else if (total[i] == '8')
        {
          SoundPlay(61); // "�ȹ鸸"
        }
        else if (total[i] == '9')
        {
          SoundPlay(62); // "���鸸"
        }
      }
      else 
      {
        if (total[i] == '1')
        {
          SoundPlay(18); // "��"
        }
        else if (total[i] == '2')
        {
          SoundPlay(19); // "�̹�"
        }
        else if (total[i] == '3')
        {
          SoundPlay(20); // "���"
        }
        else if (total[i] == '4')
        {
          SoundPlay(21); // "���"
        }
        else if (total[i] == '5')
        {
          SoundPlay(22); // "����"
        }
        else if (total[i] == '6')
        {
          SoundPlay(23); // "����"
        }
        else if (total[i] == '7')
        {
          SoundPlay(24); // "ĥ��"
        }
        else if (total[i] == '8')
        {
          SoundPlay(25); // "�ȹ�"
        }
        else if (total[i] == '9')
        {
          SoundPlay(26); // "����"
        }
      }
    }
    //�ʸ� ����
    else if (numOfDigits == 6) 
    {
      if (total[i + 1] == '0') 
      {
        if (total[i] == '1')
        {
          SoundPlay(45); // "�ʸ�"
        }
        else if (total[i] == '2')
        {
          SoundPlay(46); // "�̽ʸ�"
        }
        else if (total[i] == '3')
        {
          SoundPlay(47); // "��ʸ�"
        }
        else if (total[i] == '4')
        {
          SoundPlay(48); // "��ʸ�"
        }
        else if (total[i] == '5')
        {
          SoundPlay(49); // "���ʸ�"
        }
        else if (total[i] == '6')
        {
          SoundPlay(50); // "���ʸ�"
        }
        else if (total[i] == '7')
        {
          SoundPlay(51); // "ĥ�ʸ�"
        }
        else if (total[i] == '8')
        {
          SoundPlay(52); // "�Ƚʸ�"
        }
        else if (total[i] == '9')
        {
          SoundPlay(53); // "���ʸ�"
        }
      }
      else 
      {
        if (total[i] == '1')
        {
          SoundPlay(9); // "��"
        }
        else if (total[i] == '2')
        {
          SoundPlay(10); // "�̽�"
        }
        else if (total[i] == '3')
        {
          SoundPlay(11); // "���"
        }
        else if (total[i] == '4')
        {
          SoundPlay(12); // "���"
        }
        else if (total[i] == '5')
        {
          SoundPlay(13); // "����"
        }
        else if (total[i] == '6')
        {
          SoundPlay(14); // "����"
        }
        else if (total[i] == '7')
        {
          SoundPlay(15); // "ĥ��"
        }
        else if (total[i] == '8')
        {
          SoundPlay(16); // "�Ƚ�"
        }
        else if (total[i] == '9')
        {
          SoundPlay(17); // "����"
        }
      }
    }
    //�� ����
    else if (numOfDigits == 5) 
    {
      if (total[i] == '1')
      {
        SoundPlay(36); // "��"
      }
      else if (total[i] == '2')
      {
        SoundPlay(37); // "�̸�"
      }
      else if (total[i] == '3')
      {
        SoundPlay(38); // "�︸"
      }
      else if (total[i] == '4')
      {
        SoundPlay(39); // "�縸"
      }
      else if (total[i] == '5')
      {
        SoundPlay(40); // "����"
      }
      else if (total[i] == '6')
      {
        SoundPlay(41); // "����"
      }
      else if (total[i] == '7')
      {
        SoundPlay(42); // "ĥ��"
      }
      else if (total[i] == '8')
      {
        SoundPlay(43); // "�ȸ�"
      }
      else if (total[i] == '9')
      {
        SoundPlay(44); // "����"
      }
    }
    //õ ����
    else if (numOfDigits == 4) 
    {
      if (total[i] == '1')
      {
        SoundPlay(27); // "õ"
      }
      else if (total[i] == '2')
      {
        SoundPlay(28); // "��õ"
      }
      else if (total[i] == '3')
      {
        SoundPlay(29); // "��õ"
      }
      else if (total[i] == '4')
      {
        SoundPlay(30); // "��õ"
      }
      else if (total[i] == '5')
      {
        SoundPlay(31); // "��õ"
      }
      else if (total[i] == '6')
      {
        SoundPlay(32); // "��õ"
      }
      else if (total[i] == '7')
      {
        SoundPlay(33); // "ĥõ"
      }
      else if (total[i] == '8')
      {
        SoundPlay(34); // "��õ"
      }
      else if (total[i] == '9')
      {
        SoundPlay(35); // "��õ"
      }
    }
    //�� ����
    else if (numOfDigits == 3) 
    {
      if (total[i] == '1')
      {
        SoundPlay(18); // "��"
      }
      else if (total[i] == '2')
      {
        SoundPlay(19); // "�̹�"
      }
      else if (total[i] == '3')
      {
        SoundPlay(20); // "���"
      }
      else if (total[i] == '4')
      {
        SoundPlay(21); // "���"
      }
      else if (total[i] == '5')
      {
        SoundPlay(22); // "����"
      }
      else if (total[i] == '6')
      {
        SoundPlay(23); // "����"
      }
      else if (total[i] == '7')
      {
        SoundPlay(24); // "ĥ��"
      }
      else if (total[i] == '8')
      {
        SoundPlay(25); // "�ȹ�"
      }
      else if (total[i] == '9')
      {
        SoundPlay(26); // "����"
      }
    }
    //�� ����
    else if (numOfDigits == 2) 
    {
      if (total[i] == '1')
      {
        SoundPlay(9); // "��"
      }
      else if (total[i] == '2')
      {
        SoundPlay(10); // "�̽�"
      }
      else if (total[i] == '3')
      {
        SoundPlay(11); // "���"
      }
      else if (total[i] == '4')
      {
        SoundPlay(12); // "���"
      }
      else if (total[i] == '5')
      {
        SoundPlay(13); // "����"
      }
      else if (total[i] == '6')
      {
        SoundPlay(14); // "����"
      }
      else if (total[i] == '7')
      {
        SoundPlay(15); // "ĥ��"
      }
      else if (total[i] == '8')
      {
        SoundPlay(16); // "�Ƚ�"
      }
      else if (total[i] == '9')
      {
        SoundPlay(17); // "����"
      }
    }
    //�� ����
    else if (numOfDigits == 1) 
    {
      if (total[i] == '1')
      {
        SoundPlay(0); //"�Ͽ�"
      }
      else if (total[i] == '2')
      {
        SoundPlay(1); //"�̿�"
      }
      else if (total[i] == '3')
      {
        SoundPlay(2); //"���"
      }
      else if (total[i] == '4')
      {
        SoundPlay(3); //"���"
      }
      else if (total[i] == '5')
      {
        SoundPlay(4); //"����"
      }
      else if (total[i] == '6')
      {
        SoundPlay(5); //"����"
      }
      else if (total[i] == '7')
      {
        SoundPlay(6); //"ĥ��"
      }
      else if (total[i] == '8')
      {
        SoundPlay(7); //"�ȿ�"
      }
      else if (total[i] == '9')
      {
        SoundPlay(8); //"����"
      }
      else if (total[i] == '0')
      {
        SoundPlay(63); //"��"
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

  SoundPlay(77); //"�����Ǿ����ϴ�"
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
