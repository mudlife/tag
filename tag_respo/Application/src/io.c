#include "io.h"

void GPIO_Group_Status(GPIO_TypeDef* GPIOx)
{
  GPIOx->CR2 = 0xFF;
  GPIOx->ODR = 0x00;
  GPIOx->DDR = 0xFF;
  GPIOx->CR1 = 0xFF;
}

void io_init(void)
{
  GPIO_Group_Status(GPIOA);
  GPIO_Group_Status(GPIOB);
  GPIO_Group_Status(GPIOC);
  GPIO_Group_Status(GPIOD);
  GPIO_Group_Status(GPIOE);
}


