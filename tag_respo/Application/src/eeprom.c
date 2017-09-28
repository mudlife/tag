#include "eeprom.h"
#include "wdg.h"

u8 dfortest[6]={0,};

u8 dOwnInfoBuf[8]={0,};
u8 dCustInfoBuf[6]={0,};

void get_id_information(void)
{
  u8 i;
  u8 dInfoFlag=0;
  FLASH_SetProgrammingTime(FLASH_ProgramTime_Standard);
  FLASH_Unlock(FLASH_MemType_Data);
  while (FLASH_GetFlagStatus(FLASH_FLAG_DUL) == RESET)
  {
  }
  dInfoFlag=FLASH_ReadByte(FLASH_DATA_EEPROM_START_PHYSICAL_ADDRESS+0x10);
 // dfortest=FLASH_ReadByte(FLASH_DATA_EEPROM_START_PHYSICAL_ADDRESS+0x11);
  if(dInfoFlag==0x01)
  {
    for(i=0;i<6;i++)
    {
      dfortest[i]=FLASH_ReadByte(FLASH_DATA_EEPROM_START_PHYSICAL_ADDRESS+0x20+i);
      dCustInfoBuf[i]=FLASH_ReadByte(FLASH_DATA_EEPROM_START_PHYSICAL_ADDRESS+0x20+i);
    }
    for(i=0;i<8;i++)
    {
      dOwnInfoBuf[i]=FLASH_ReadByte(FLASH_DATA_EEPROM_START_PHYSICAL_ADDRESS+0x30+i);
    }
  }
  else
  {
    for(i=0;i<6;i++)
    {
      dCustInfoBuf[i]=0xAA;
    }
    for(i=0;i<8;i++)
    {
      dOwnInfoBuf[i]=0xAA;
    }
  }
  FLASH_Lock(FLASH_MemType_Data);
}

void write_eeprom_halfword(u32 address,u16 value)
{
  //FLASH_SetProgrammingTime(FLASH_ProgramTime_Standard);
  FLASH_Unlock(FLASH_MemType_Data);
  while (FLASH_GetFlagStatus(FLASH_FLAG_DUL) == RESET)
  {
    wwdg_feed();
  }
  FLASH_EraseByte(address);
  while (FLASH_GetFlagStatus(FLASH_FLAG_EOP) == RESET)
  {
    wwdg_feed();
  }
  FLASH_EraseByte(address+1);
  while (FLASH_GetFlagStatus(FLASH_FLAG_EOP) == RESET)
  {
    wwdg_feed();
  }
  FLASH_ProgramByte(address,(u8)(value&0xFF));
  while (FLASH_GetFlagStatus(FLASH_FLAG_EOP) == RESET)
  {
    wwdg_feed();
  }
  FLASH_ProgramByte(address+1,(u8)((value&0xFF00)>>8));
  while (FLASH_GetFlagStatus(FLASH_FLAG_EOP) == RESET)
  {
    wwdg_feed();
  }
  FLASH_Lock(FLASH_MemType_Data);
}

  u8 RXERROR=0;
  u8 TXERROR=1;
  u8 CANNOTSLEEP=2;
  u8 CANNOTRESET=3;
  u8 SPIERROR=4;

u16 dRxErrorCnt=0;
u16 dTxErrorCnt=0;
u16 dCanNotSleepCnt=0;
u16 dCanNotResetCnt=0;
u16 dSPIErrorCnt=0;

#define RXERROR_BASEADDRESS FLASH_DATA_EEPROM_START_PHYSICAL_ADDRESS+0x40
#define TXERROR_BASEADDRESS FLASH_DATA_EEPROM_START_PHYSICAL_ADDRESS+0x42
#define CANNOTSLEEP_BASEADDRESS FLASH_DATA_EEPROM_START_PHYSICAL_ADDRESS+0x44
#define CANNOTRESET_BASEADDRESS FLASH_DATA_EEPROM_START_PHYSICAL_ADDRESS+0x46
#define SPIERROR_BASEADDRESS FLASH_DATA_EEPROM_START_PHYSICAL_ADDRESS+0x48

void save_error_msg(u8 type)
{
  switch(type){
    case 0:
      dRxErrorCnt=(FLASH_ReadByte(RXERROR_BASEADDRESS+1)<<8)+FLASH_ReadByte(RXERROR_BASEADDRESS)+1;
      if(dRxErrorCnt<=0xFF00)
      {
        write_eeprom_halfword(RXERROR_BASEADDRESS,dRxErrorCnt);
      }
      dRxErrorCnt=(FLASH_ReadByte(RXERROR_BASEADDRESS+1)<<8)+FLASH_ReadByte(RXERROR_BASEADDRESS)+1;
      break;
  case 1:
    dTxErrorCnt=(FLASH_ReadByte(TXERROR_BASEADDRESS+1)<<8)+FLASH_ReadByte(TXERROR_BASEADDRESS)+1;
      if(dTxErrorCnt<=0xFF00)
      {
        write_eeprom_halfword(TXERROR_BASEADDRESS,dTxErrorCnt);
      }
      break;
  case 2:
    dCanNotSleepCnt=(FLASH_ReadByte(CANNOTSLEEP_BASEADDRESS+1)<<8)+FLASH_ReadByte(CANNOTSLEEP_BASEADDRESS)+1;
      if(dCanNotSleepCnt<=0xFF00)
      {
        write_eeprom_halfword(CANNOTSLEEP_BASEADDRESS,dCanNotSleepCnt);
      }
      break;
  case 3:
    dCanNotResetCnt=(FLASH_ReadByte(CANNOTRESET_BASEADDRESS+1)<<8)+FLASH_ReadByte(CANNOTRESET_BASEADDRESS)+1;
      if(dCanNotResetCnt<=0xFF00)
      {
        write_eeprom_halfword(CANNOTRESET_BASEADDRESS,dCanNotResetCnt);
      }
      break;
  case 4:
    dSPIErrorCnt=(FLASH_ReadByte(SPIERROR_BASEADDRESS+1)<<8)+FLASH_ReadByte(SPIERROR_BASEADDRESS)+1;
      if(dSPIErrorCnt<=0xFF00)
      {
        write_eeprom_halfword(SPIERROR_BASEADDRESS,dSPIErrorCnt);
      }
      break;
  default:break;
  }
}

