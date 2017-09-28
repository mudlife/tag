#include "sysclk.h"

void clock_init(void)
{
  CLK_DeInit();
  CLK_HSICmd(ENABLE);
  CLK_SYSCLKSourceConfig(CLK_SYSCLKSource_HSI);
  CLK_SYSCLKDivConfig(CLK_SYSCLKDiv_1);
}


