#include "timer.h"
#include "finder.h"
#include "ble_proto.h"
#define BTN_POLLING_ITVL        (10) // ms

//static u8 dTimeCnt=0;
//u8 dTimeOutFlag=0;

u8 btn_poll_cntdn = BTN_POLLING_ITVL;
u8 finder_base_cntdn = FINDER_OPER_TIME_BASE;
/* 16M/32768=488Hz
    system will raise timer 4 update interrupt every 0.5 second
    if TIM4_PERIOD=243,(243+1/488)
16M /16 = 1MHz
1ms
   TIM4_PERIOD=999,(999+1/1 000 000)
 
*/
void timer4_cfg(void)
{
    CLK_PeripheralClockConfig(CLK_Peripheral_TIM4, ENABLE);//Enable TIM4 CLK
    TIM4_TimeBaseInit(TIM4_Prescaler_16, TIM4_PERIOD);//Time base configuration
    TIM4_ClearFlag(TIM4_FLAG_Update);//Clear TIM4 update flag
    TIM4_ITConfig(TIM4_IT_Update, ENABLE);//Enable update interrupt
    enableInterrupts();//enable interrupts
    TIM4_Cmd(ENABLE);//Enable TIM4
}

/*TIM4 Update/Overflow/Trigger Interrupt routine.*/
INTERRUPT_HANDLER(TIM4_UPD_OVF_TRG_IRQHandler,25)
{
    /* In order to detect unexpected events during development,
       it is recommended to set a breakpoint on the following instruction.
    */
//    if(dTimeCnt++>=240)
//    {
//        dTimeCnt=0;
//        dTimeOutFlag=1;
//    }
    
    
       /* Manage ADV interval */
    ble_manage_adv_interval();

    /* Poll button state when necessary */
    if (--btn_poll_cntdn == 0) {
        btn_poll_cntdn = BTN_POLLING_ITVL;
//        user_action = btn_check_action();

    }
    
    /* Process finder's operation countdown */
    if ((finder_oper_cntdn != 0) && (--finder_base_cntdn == 0)) {
        finder_base_cntdn = FINDER_OPER_TIME_BASE;
        if (--finder_oper_cntdn == 0) {
            finder_is_oper_timeout = 1;
        }
    }
    TIM4_ClearITPendingBit(TIM4_IT_Update);
}