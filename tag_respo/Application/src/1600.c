#include "1600.h"
#include "stm8l15x_spi.h"
#include "sleep.h"
#include "se2438t.h"

static const u8 dRegisterTable[][2] = {

//{34,0x60},//{34,0x13},{35,0xc0},
//{0,0x6f},{1,0xe9},{2,0x57},{3,0x0d},{4,0xc4},
//{9,0x14},{11,0x1b},{17,0x5a},{18,0x38},{19,0x00},
//{46,0x01},{49,0x5b},{53,0x00},
//{64,0x78},{72,0xA5},{73,0xC3},{74,0x96},{75,0xFF},
//{76,0xAA},{77,0xCC},{78,0x99},{79,0xF0},{15,0x00},
//{14,0x00},{8,0x25},{10,0x45},{16,0x84},{27,0xa4},
//{47,0x15},{50,0x15},{51,0x14},{34,0x13},{35,0xC0},{255, 255} 
  
  {0x00,0x6F},{0x01,0xE9},{0x02,0x57},{0x03,0x0D},{0x04,0xC4},{0x08,0x25},
{0x09,0x14},{0x0A,0x45},{0x0B,0x1B},{0x10,0x84},{0x11,0x5A},{0x12,0x08},
{0x13,0x00},{0x1B,0xA4},{0x22,0x13},{0x23,0xC0},{0x2E,0x01},{0x2F,0x15},
{0x31,0x5B},{0x32,0x15},{0x33,0x14},{0x35,0x00},{0x40,0x20},{0x41,0x20},
{0x48,0xA5},{0x49,0xC3},{0x4A,0x96},{0x4B,0xFF},{0x4C,0xAA},{0x4D,0xCC},
{0x4E,0x99},{0x4F,0xF0},{0x0F,0x00},{0x0E,0x00},{255, 255} 

};
static const u8 dRegisterTable1[][2] = {
    {0,  0x6F},
    {1,  0xE8},
    {2,  0x56},
    {3,  0x0D},
    {4,  0xC4}, // C4
    {8,  0x25}, // 05 | 25
    {9,  0x41},
    {10, 0x57},
    {11, 0x3B},
    {16, 0x84},
    {17, 0x7A}, // 7B | 5A
    {18, 0x08},
    {19, 0x08},
    {27, 0xB4},
    {34, 0x12},
    {35, 0x00},
    {45, 0x06},
    {46, 0x01},
    {47, 0x15},
    {49, 0x5B},
    {50, 0x15},
    {51, 0x14},
    {52, 0x1A},
    {53, 0x00},
    {64, 0x0A},
    {65, 0x00},
    {72, 0xBE},
    {73, 0xD6}, 
    {78, 0x8E},
    {79, 0x89}, 
    {81, 0x44},
    {82, 0x31},  
    {66, 0x60},
    {67, 0xA0},
    {68, 0x80},    
    {255, 255} 
};
extern u8 dCustInfoBuf[];
extern volatile u8 send_channel;
void _1600_io_init(void)
{
  GPIO_Init(IO_CE_SRC, IO_CE_PIN, GPIO_Mode_Out_PP_High_Fast);
  GPIO_Init(IO_CS_SRC, IO_CS_PIN, GPIO_Mode_Out_PP_High_Fast);
  GPIO_Init(IO_PKT_SRC, IO_PKT_PIN, GPIO_Mode_In_PU_No_IT);
  _CS__0;
  _CE__0;
  delay_ms(2);
  _CS__1;
  _CE__1;
  delay_ms(1);
}

void wakeup_1600(void)
{
  u8 i=0;
  _CS__0;
  delay_ms(2);
  _CS__1;
  delay_ms(2);
}

void switch_mode(u8 mode)
{
  if(mode==1)
  {
   write1600(65,0x20);
   write1600(64,0x20);//preamble length-2bytes,syncword length-2bytes,trailer length-4bits
   write1600(72,0xA5);
   write1600(73,0xC3);
  }else{   //ble 
    write1600(65,0x00);
    write1600(64,0x0A);//preamble length-1bytes,syncword length-4bytes,trailer length-8bits
    write1600(72,0xBE);
    write1600(73,0xD6);
  }
}


void _1600_registers_init(u8 mode)
{
  u8 i=0,cnt=0,errorcnt=0,j=0,errorflag=0;
  if(mode == 1){
  for(i=0;dRegisterTable[i][0] != 255;)
  {
    IWDG_ReloadCounter();
    write1600(dRegisterTable[i][0],dRegisterTable[i][1]);
    if(dRegisterTable[i][1]!=read1600(dRegisterTable[i][0]))
    {
      if(errorcnt++>=10)
      {
        errorflag=1;
        break;
      }
    }
    else
    {
      i++;
      errorcnt=0;
    }
  }
  write1600(81,0x44);//crc bits should be right

 
  }else{//BLE
  for(i=0;dRegisterTable[i][0] != 255;)
  {
    IWDG_ReloadCounter();
    write1600(dRegisterTable1[i][0],dRegisterTable1[i][1]);
    if(dRegisterTable1[i][1]!=read1600(dRegisterTable1[i][0]))
    {
      if(errorcnt++>=10)
      {
        errorflag=1;
        break;
      }
    }
    else
    {
      i++;
      errorcnt=0;
    }
  }
   write1600(81,0x43);//crc bits should be right
  }
  
  
  write1600_bit(82,5,1);//enable length mask
  write1600_bit(33,1,1);//enable pkt flag mask
  
 #if 0 
  if(errorflag)
  {//init1600 failed send msg once
    _1600_clean_tx_fifo();
    write1600(100,9);
    write1600(100,0x38);
    write1600(100,3);
    for(i=0;i<=5;i++)
    {
      write1600(100,dCustInfoBuf[i]);
    }
    write1600(100,0);
    write1600(15,send_channel);
    se2438t_enter_T_mode();
    _1600_tx_on();
    while(!_1600_pkt_status())
    {
      if(++cnt>=50)
      {
        cnt=0;
        break;
      }
    }
    delay_1us();
    delay_1us();
    se2438t_enter_all_off_mode();
    _CE__0;
    for(i=0;i<40;i++)
    {
      _CS__TOGGLE;
      IWDG_ReloadCounter();
      sleep(3);
      RTC_WakeUpCmd(DISABLE);
      CLK_PeripheralClockConfig(CLK_Peripheral_RTC, DISABLE);
    }
    WWDG_SWReset();
  }
#endif
  #if 0//tx direct
	write1600(14,18);//0X12
        write1600(15,76);//0X4c
        write1600(1,233);//0Xe9
        write1600(2,87);//0X57
        write1600(3,13);//0X0d
        write1600(4,196);//0Xc4
        write1600(8,37);//0X25
        write1600(9,20);//0X14
        write1600(10,69);//0X45
        write1600(11,27);//0X1b
        write1600(16,132);//0X84
        write1600(17,90);//0X5A
        write1600(18,8);//0x08
        write1600(19,0);//0x00
        write1600(49,91);//0x5b
        write1600(27,164);//0xa4
        write1600(34,19);//0x13
        write1600(35,192);//0xC0
        write1600(46,1);//0x01
        write1600(47,21);//0x15
        write1600(49,91);//0x5b
        write1600(50,21);//0x15
        write1600(51,20);//0x14
        write1600(52,26);//0x1A
        write1600(53,0);//0x00
        write1600(64,120);//0x78
        write1600(72,97);//0x61
        write1600(73,7);//0x07
        write1600(74,23);//0x17
        write1600(75,148);//0x94
        write1600(76,237);//0xED
        write1600(77,39);//0x27
        write1600(78,117);//0x75
        write1600(1,249);//0xF9
        write1600(65,1);//0x01
        write1600(68,176);//0xB0
        write1600(22,131);
        write1600(14,0x05);//0x04
        write1600(15,0x4B);//0xCb
        write1600(18,0x38);
        se2438t_enter_T_mode();
	while(1)
	{
		wwdg_feed();
	}
	#endif
	#if 0//rx direct
	write1600(14,18);
        write1600(15,76);
        write1600(1,233);
        write1600(2,87);
        write1600(3,13);
        write1600(4,196);
        write1600(8,37);
        write1600(9,20);
        write1600(10,69);
        write1600(11,27);
        write1600(16,132);
        write1600(17,90);
        write1600(18,0x08);
        write1600(19,0);
        write1600(49,91);
        write1600(27,164);
        write1600(34,19);
        write1600(35,192);
        write1600(46,1);
        write1600(47,21);
        write1600(50,21);
        write1600(51,20);
        write1600(52,0x1E);
        write1600(53,0);
        write1600(64,120);
        write1600(72,97);
        write1600(73,7);
        write1600(74,23);
        write1600(75,148);
        write1600(76,237);
        write1600(77,39);
        write1600(78,117);
        write1600(1,249);
        write1600(65,1);
        write1600(68,176);
        write1600(22,0x83);
        write1600(14,0x20);//0x04
        write1600(15,0xCD);//0xCb
        se2438t_enter_high_gain_R_mode();
	while(1)
	{
		wwdg_feed();
	}
	#endif
}

volatile u8 read1600(u8 reg)
{
  u8 ret;
  _CS__0;
  SPI_ReadWriteByte(reg | 0x80);
  ret = SPI_ReadWriteByte(0xFF);
  _CS__1;
  return ret;
}

void write1600(u8 reg,u8 value)
{
  _CS__0;
  SPI_ReadWriteByte(reg);
  SPI_ReadWriteByte(value);
  _CS__1;
}

void write1600_bit(u8 reg,u8 bit,u8 value)
{
  u8 reg_val,bit_num;
  bit_num = 0x1 << bit;
  reg_val = read1600(reg);
  if(value == 1)
  {
    reg_val |= bit_num;
  }
  else
  {
    reg_val &= ~bit_num;
  }
  write1600(reg,reg_val);
}

u8 read1600_bit(u8 reg,u8 bit)
{
  u8 reg_val,bit_num;
  bit_num = 0x1 << bit;
  reg_val = read1600(reg);
  if((reg_val&bit_num) == 0x0)
  {
    return 0;
  }
  else
  {
    return 1;
  }
}

void _1600_clean_rx_fifo(void)
{
  write1600_bit(105,7,1);
}

void _1600_rx_on(void)
{
  write1600(52,0x1e);
//  write1600_bit(14,0,0);
  write1600_bit(15,7,1);
}

u8 _1600_pkt_status(void)
{
  return read1600_bit(97,6);
}

void _1600_clear_pkt(void)
{
  write1600_bit(37,1,1);
}

u8 _1600_crc_verify_result(void)
{
  return read1600_bit(96,7);
}

void _1600_clean_tx_fifo(void)
{
  write1600_bit(104,7,1);
}

void _1600_frequency_channel(u8 value)
{
  u8 reg_value,reg15;
  reg15 = read1600(15);
  if(value <= 78)
  {
    reg_value = (reg15&0x80)+value;
    write1600(15,reg_value);
  }
}

void _1600_tx_on(void)
{
  write1600(52,0x1a);
//  write1600_bit(15,7,0);
  write1600_bit(14,0,1);
}

void wl1600_write_fifo(u8 reg, u8 *buf, u8 len)
{
  u8 ch;
    _CS__0;
    SPI_ReadWriteByte(reg);
    while (len--) {
       ch = *buf;
        SPI_ReadWriteByte(*buf++);
    }
    _CS__1;   
}

void wl1600_read_fifo(u8 reg, u8 *buf, u8 len)
{
    _CS__0;
    SPI_ReadWriteByte(reg | 0x80);
    while (len--) {
        *buf++ = SPI_ReadWriteByte(0xFF);
    }
     _CS__1;
}