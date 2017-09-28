#include "spi.h"

void spi_init(void)
{
  CLK_PeripheralClockConfig(CLK_Peripheral_SPI1, ENABLE);

  GPIO_ExternalPullUpConfig(GPIOB,MOSI_PIN | CLK_PIN, ENABLE);
  GPIO_Init( GPIOB, MISO_PIN, GPIO_Mode_In_FL_No_IT  );

  SPI_Init(SPI1, SPI_FirstBit_MSB, SPI_BaudRatePrescaler_2, SPI_Mode_Master,SPI_CPOL_Low, SPI_CPHA_2Edge, SPI_Direction_2Lines_FullDuplex,SPI_NSS_Soft, 0x07);

  SPI_Cmd(SPI1, ENABLE);
}

u8 SPI_ReadWriteByte(u8 Data)
{
  while (SPI_GetFlagStatus(SPI1, SPI_FLAG_TXE) == RESET)
  {}
  SPI_SendData(SPI1, Data);
  while (SPI_GetFlagStatus(SPI1, SPI_FLAG_RXNE) == RESET)
  {}
  return SPI_ReceiveData(SPI1);
}
