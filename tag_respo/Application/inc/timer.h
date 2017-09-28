#ifndef _timer_h_
#define _timer_h_

#include "stm8l15x.h"

#define TIM4_PERIOD       999
#define DISABLE_BLE_TIMER() TIM4_Cmd(DISABLE)
#define ENABLE_BLE_TIMER() TIM4_Cmd(ENABLE)
void timer4_cfg(void);

#endif
