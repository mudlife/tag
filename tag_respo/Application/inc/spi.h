#ifndef _spi_h_
#define _spi_h_

#include "stm8l15x.h"

#define MOSI_CLK_SRC	GPIOB
#define MOSI_PIN	GPIO_Pin_6
#define MISO_CLK_SRC	GPIOB
#define MISO_PIN	GPIO_Pin_7
#define CLK_CLK_SRC	GPIOB
#define CLK_PIN	GPIO_Pin_5

u8 SPI_ReadWriteByte(u8 Data);
void spi_init(void);

#endif
