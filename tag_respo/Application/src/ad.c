#include "ad.h"

uint32_t ADCdata = 1;
uint16_t ADCRefdata =1;
uint32_t PotVoltage = 1;
static uint8_t flag=1;

void ADC_Config(void)
{
  /* Initialise and configure ADC1 */
  ADC_DeInit(ADC1);
  CLK_PeripheralClockConfig(CLK_Peripheral_ADC1, ENABLE);
  ADC_Init(ADC1, ADC_ConversionMode_Single, ADC_Resolution_12Bit, ADC_Prescaler_2);
  
  ADC_SamplingTimeConfig(ADC1, ADC_Group_SlowChannels, ADC_SamplingTime_4Cycles);
  ADC_SamplingTimeConfig(ADC1, ADC_Group_FastChannels, ADC_SamplingTime_4Cycles);
  ADC_Cmd(ADC1, ENABLE);
  ADC_VrefintCmd(ENABLE);
  ADC_ChannelCmd(ADC1, ADC_Channel_4, ENABLE);
  ADC_ChannelCmd(ADC1, ADC_Channel_Vrefint, ENABLE);
  
  /* Enable End of conversion ADC1 Interrupt */
  ADC_ITConfig(ADC1, ADC_IT_EOC, ENABLE);

  /* Start ADC1 Conversion using Software trigger*/
  ADC_SoftwareStartConv(ADC1);
  
  enableInterrupts();
}

uint32_t get_voltage(void)
{
  PotVoltage = (uint32_t)1224*4096/(uint32_t)ADCdata;
  ADC_Cmd(ADC1, DISABLE);
  return PotVoltage;
}
/**
  * @brief ADC1/Comparator Interrupt routine.
  * @param  None
  * @retval None
  */
INTERRUPT_HANDLER(ADC1_COMP_IRQHandler, 18)
{
  /* In order to detect unexpected events during development,
     it is recommended to set a breakpoint on the following instruction.
  */
  /* Get ADC convertion value */
    if(ADC_GetITStatus(ADC1,ADC_IT_EOC)==SET){
      ADCdata = ADC_GetConversionValue(ADC1);
      ADC_ClearITPendingBit(ADC1,ADC_IT_EOC);
    }
}
