#include "se2438t.h"

void se2438t_init(void)
{
  GPIO_Init(CSD_SRC,CSD_PIN,GPIO_Mode_Out_PP_High_Fast);
  GPIO_Init(CTX_SRC,CTX_PIN,GPIO_Mode_Out_PP_High_Fast);
  GPIO_Init(CPS_SRC,CPS_PIN,GPIO_Mode_Out_PP_High_Fast);
  GPIO_Init(CRX_SRC,CRX_PIN,GPIO_Mode_Out_PP_High_Fast);

  _CSD__0;
  _CTX__0;
  _CPS__0;
  _CRX__0;
}

void se2438t_enter_high_gain_R_mode(void)
{
  _CSD__1;
  _CTX__0;
  _CPS__1;
  _CRX__1;
}

void se2438t_enter_all_off_mode(void)
{
  _CSD__0;
  _CTX__0;
  _CPS__0;
  _CRX__0;
}

void se2438t_enter_T_mode(void)
{
  _CSD__1;
  _CTX__1;
  _CPS__1;
  _CRX__0;
}