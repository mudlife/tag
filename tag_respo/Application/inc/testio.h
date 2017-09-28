#ifndef _testio_h
#define _testio_h

#include "stm8l15x.h"

#define TESTIO_PORT     GPIOB
#define TESTIO_PIN      GPIO_Pin_0

#define TESTIO_1        TESTIO_PORT->ODR|=TESTIO_PIN
#define TESTIO_0        TESTIO_PORT->ODR&=~TESTIO_PIN
#define TESTIO_TOGGLE   TESTIO_PORT->ODR^=TESTIO_PIN
void testio_init(void);

#endif
