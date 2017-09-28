#include "sleep.h"
#include "wdg.h"
#include "random.h"
#include "testio.h"

void sleep(u8 space)
{
    TIM4_Cmd(DISABLE);
    CLK_PeripheralClockConfig(CLK_Peripheral_ADC1, DISABLE);
    PWR_PVDCmd(DISABLE);
    CLK_RTCClockConfig(CLK_RTCCLKSource_LSI,CLK_RTCCLKDiv_1);
    CLK_PeripheralClockConfig(CLK_Peripheral_RTC, ENABLE);
    RTC_WakeUpCmd(DISABLE);
    RTC_WakeUpClockConfig(RTC_WakeUpClock_CK_SPRE_16bits);
    RTC_ITConfig(RTC_IT_WUT, ENABLE);
    enableInterrupts();
    RTC_SetWakeUpCounter(space);
    RTC_WakeUpCmd(ENABLE);
    PWR_FastWakeUpCmd(ENABLE);
    PWR_UltraLowPowerCmd(ENABLE);
    IWDG_ReloadCounter();
    sim();
    halt();
#if 1
    RTC_WakeUpCmd(DISABLE);
    IWDG_ReloadCounter();
    CLK_RTCClockConfig(CLK_RTCCLKSource_LSI,CLK_RTCCLKDiv_1);
    CLK_PeripheralClockConfig(CLK_Peripheral_RTC, ENABLE);
    RTC_WakeUpClockConfig(RTC_WakeUpClock_RTCCLK_Div2);
    RTC_ITConfig(RTC_IT_WUT, ENABLE);
    enableInterrupts();
    RTC_SetWakeUpCounter(get_rand_time()%4000);// 16393=1s  get_rand_time()
    RTC_WakeUpCmd(ENABLE);
    PWR_FastWakeUpCmd(ENABLE);
    PWR_UltraLowPowerCmd(ENABLE);
    IWDG_ReloadCounter();
    sim();
    halt();
#endif
}

void after_wakeup(void)
{
  RTC_WakeUpCmd(DISABLE);
  CLK_PeripheralClockConfig(CLK_Peripheral_RTC, DISABLE);
  TIM4_Cmd(ENABLE);
  PWR_PVDCmd(ENABLE);
}


/*RTC / CSS_LSE Interrupt routine. */
INTERRUPT_HANDLER(RTC_CSSLSE_IRQHandler,4)
{
    /* In order to detect unexpected events during development,
       it is recommended to set a breakpoint on the following instruction.
    */
    RTC_ClearITPendingBit(RTC_IT_WUT);
}