#include "wdg.h"
#include "stm8l15x_wwdg.h"

void wwdg_init(void)
{
  CLK_LSICmd(ENABLE);//Enable LSI clock
  while (CLK_GetFlagStatus(CLK_FLAG_LSIRDY) == RESET)
  {}//Wait for LSI clock to be ready
  WWDG_SetWindowValue(0x7F);
  WWDG_Enable(0x41);
}

void wwdg_feed(void)
{
  WWDG_SetCounter(0x41);
}
