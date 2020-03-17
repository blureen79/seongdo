#include "ndevice_rnd.h"
#include "ndevice_util.h"

static stRNDoperate* pRNDoperate_g = ((stRNDoperate*)ND_RND_BASE);

/*
* @brief : Initialize RND
* @ret : FALSE->Failed to connect, TRUE->Succeed to conned
*/
void nd_rnd_init(void)
{
  pRNDoperate_g->stop = 1;

  ND_RND_EVNT_VALRDY = 0;           //Clear event

  pRNDoperate_g->start = 1;  
}

/*
* @brief : get generted value
*/
uint8_t nd_get_randomed_num(void)
{
  while(ND_RND_EVNT_VALRDY == 0) {};
  ND_RND_EVNT_VALRDY = 0;           //Clear event

  return (uint8_t)ND_RND_VALUE;
}
