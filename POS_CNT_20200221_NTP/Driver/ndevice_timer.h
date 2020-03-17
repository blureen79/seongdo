#ifndef __NDEVICE_TIMER
#define __NDEVICE_TIMER

#include "nrf_drv_timer.h"

extern void nd_timer_init(void);
extern void imu_timer_start(void);
extern void imu_timer_stop(void);
#endif
