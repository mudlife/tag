#ifndef _se2438t_h_
#define _se2438t_h_

#include "stm8l15x.h"

#define CSD_SRC GPIOD
#define CSD_PIN GPIO_Pin_0
#define CTX_SRC GPIOA
#define CTX_PIN GPIO_Pin_3
#define CPS_SRC GPIOB
#define CPS_PIN GPIO_Pin_2
#define CRX_SRC GPIOB
#define CRX_PIN GPIO_Pin_1

#define _CSD__1	CSD_SRC->ODR |=CSD_PIN
#define _CSD__0	CSD_SRC->ODR &=~CSD_PIN
#define _CTX__1	CTX_SRC->ODR|=CTX_PIN
#define _CTX__0	CTX_SRC->ODR&=~CTX_PIN
#define _CPS__1	CPS_SRC->ODR|=CPS_PIN
#define _CPS__0	CPS_SRC->ODR&=~CPS_PIN
#define _CRX__1	CRX_SRC->ODR|=CRX_PIN
#define _CRX__0	CRX_SRC->ODR&=~CRX_PIN

void se2438t_init(void);
void se2438t_enter_high_gain_R_mode(void);
void se2438t_enter_all_off_mode(void);
void se2438t_enter_T_mode(void);

#endif
