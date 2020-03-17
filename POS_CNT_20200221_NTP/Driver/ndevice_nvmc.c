#include <stdbool.h>
#include "nrf_delay.h"
#include "ndevice_nvmc.h"
#include "SEGGER_RTT.h"

/*
*@ brief : UICR영역 읽기
*/
uint32_t getUICR(uint32_t addr)
{
  uint32_t ret = 0;

  if(ND_NVMC_CONFIG != DEF_NVMC_CONFIG_REN)
  {
    nd_nvmc_cfg(DEF_NVMC_CONFIG_REN);
  }

  ret = NRF_UICR->CUSTOMER[addr];
  
  return ret;
}

/*
*@ brief : UICR영역 쓰기
*/
void setUICR(uint32_t addr, uint32_t data)
{
  if(getUICR(addr) != 0xFFFFFFFF) 
  {
    SEGGER_RTT_printf(0, "Erase UICR\n");
    nd_uicr_erase();
  }

  if(ND_NVMC_CONFIG != DEF_NVMC_CONFIG_WEN)
  {
    SEGGER_RTT_printf(0, "Set WEN\n");
    nd_nvmc_cfg(DEF_NVMC_CONFIG_WEN);
  }  

  SEGGER_RTT_printf(0, "Write UICR\n");
  NRF_UICR->CUSTOMER[addr] = data;

  //nd_nvmc_cfg(DEF_NVMC_CONFIG_REN);
}

/*
* @brief : check nvmc is busy
@ret : TRUE:Busy, FALSE:Ready to use
*/
bool is_busy_nvmc(void)
{
  bool ret = true;
  
  if(ND_NVMC_READY != 0x00000000UL) ret = false;
  
  return ret;
}

/*
* @brief : Configure NVMC
*/
void nd_nvmc_cfg(uint32_t cfg)
{
  ND_NVMC_CONFIG = cfg;

  while(is_busy_nvmc() == true) {}
}

/*
* @brief : write data
*/
void nd_nvmc_write(uint32_t addr, uint32_t* pData, uint8_t len)
{
  uint32_t i;
  uint32_t* target;

  //if(nd_nvmc_read(addr) != 0xFFFFFFFF) nd_nvmc_erase(addr);

  if(ND_NVMC_CONFIG != DEF_NVMC_CONFIG_WEN)
  {
    nd_nvmc_cfg(DEF_NVMC_CONFIG_WEN);
  }

  target = (uint32_t*)(addr);

  for(i=0; i<len; ++i) target[i] = pData[i];

  //nd_nvmc_cfg(DEF_NVMC_CONFIG_REN);
}

/*
* @brief : read data from NVMC
*/
uint32_t nd_nvmc_read(uint32_t addr)
{
  uint32_t data;
  uint32_t* target;

  if(ND_NVMC_CONFIG != DEF_NVMC_CONFIG_REN)
  {
    nd_nvmc_cfg(DEF_NVMC_CONFIG_REN);
  }
  
  target = (uint32_t*)(addr);
  data = *target;

  return data;
}

/*
* @brief : erase page data
*/
void nd_nvmc_erase(uint32_t addr)
{
  if(ND_NVMC_CONFIG != DEF_NVMC_CONFIG_EEN)
  {
    nd_nvmc_cfg(DEF_NVMC_CONFIG_EEN);
  }

  ND_NVMC_ERPAGE = addr;

  while(is_busy_nvmc() == true) {}
}

/*
* @brief : erase uicr page
*/
void nd_uicr_erase(void)
{
  if(ND_NVMC_CONFIG != DEF_NVMC_CONFIG_EEN)
  {
    nd_nvmc_cfg(DEF_NVMC_CONFIG_EEN);
  }

  ND_NVMC_ERUICR = 1UL;

  while(is_busy_nvmc() == true) {}
}

