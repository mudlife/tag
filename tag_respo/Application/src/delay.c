#include "delay.h"
#include "wdg.h"

void delay_ms(u16 mtime)
{
  u32 i,j;
  for(i=0;i<mtime*100;i++)
  {
    for(j=0;j<1;j++);
    IWDG_ReloadCounter();
  }
}

void delay_1us(void)
{
  nop();
  nop();
  nop();
  nop();
}

void delay_us(u16 t)
{
  while(t--){
    delay_1us();
  }
}



