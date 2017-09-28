#include "testio.h"

void testio_init(void)
{
  GPIO_Init(TESTIO_PORT,TESTIO_PIN,GPIO_Mode_Out_PP_High_Fast);
  TESTIO_0;
}