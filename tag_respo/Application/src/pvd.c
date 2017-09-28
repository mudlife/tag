#include "pvd.h"


u8 dVDDlow=0;

void pvd_config(void)
{
  PWR_PVDLevelConfig(PWR_PVDLevel_2V45);
  PWR_PVDCmd(ENABLE);
  PWR_PVDITConfig(ENABLE);
}


/**
  * @brief External IT PORTE/F and PVD Interrupt routine.
  * @param  None
  * @retval None
  */
INTERRUPT_HANDLER(EXTIE_F_PVD_IRQHandler, 5)
{
  /* In order to detect unexpected events during development,
     it is recommended to set a breakpoint on the following instruction.
  */
  if (PWR_PVDGetITStatus() != RESET)
  {
    PWR_PVDClearITPendingBit();
    if (PWR_GetFlagStatus(PWR_FLAG_PVDOF) != RESET)
    {/* Falling Direction Detected,VDD<PVD_level */
      dVDDlow=1;
    }
    else
    {/* Rising Direction Detected,VDD>PVD_level*/
      dVDDlow=0;
    }
  }
}