#include "stm8l15x.h"
#include "eeprom.h"
#include "wdg.h"
#include "1600.h"
#include "se2438t.h"
#include "sysclk.h"
#include "io.h"
#include "random.h"
#include "pvd.h"
#include "sleep.h"
#include "timer.h"
#include "testio.h"
#include "string.h"
#include "ad.h"

#include "ble_pkts.h"
#include "ble_proto.h"
#include "finder.h"

extern u8 dfortest;

//x  xxx  xxxx  xxxx  xxxx
//|   |     |     |         |
//c/o pa   times space    test

volatile u8 send_channel=78;
static volatile u8 receive_channel=23;
#if 1
static u8 send_pa[6]={0xF8,0xF8,0xD8,0xA8,0x78,0x38};//the first object of array is for close distanse mode,others are for normal distance mode
static u8 send_space[16]={9,2,3,0x71,0X93,0x35,0x77,0x00,0x00,10,11,12,13,14,15,16};
static u8 send_times[16]={1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,16};


extern uint32_t PotVoltage;

static volatile u8 t_info=0;//t_info=0,transmit the custom message(key+id),t_info=1,transmit all message(own message+key+id)
u8 t_space=0;
static volatile signed char t_times=6;
static volatile u8 t_pa=0;
static volatile u8 t_para_test=0;
static volatile u16 totaltimes=0;

static u32 timecnt=0;

static bool dActiveFlag=FALSE;
u8 loop_count = 9;
extern u8 dVDDlow;
extern u8 dOwnInfoBuf[];
extern u8 dCustInfoBuf[];

static void abnormal_cheke_process(void);
static void detect_process(void);
static void output_message(void);
#endif

void iwdg_init(void)
{
  CLK_LSICmd(ENABLE);//Enable LSI clock
  while (CLK_GetFlagStatus(CLK_FLAG_LSIRDY) == RESET)
  {}//Wait for LSI clock to be ready
  do
  {
	FLASH->DUKR = 0xae; // 写入第一个密钥
	FLASH->DUKR = 0x56; // 写入第二个密钥
  }
  while((FLASH->IAPSR & 0x08) == 0);//e2解锁
  FLASH->CR2 |= 0x80;
  OPT->WDG |= 0x02;//低功耗停止看门狗计数
  while((FLASH->IAPSR & 0x04) == 0)//写入完成
  {
	;
  }
    IWDG_Enable();//记得先使能
   IWDG_WriteAccessCmd(IWDG_WriteAccess_Enable);
   IWDG_SetPrescaler(IWDG_Prescaler_256);
   IWDG_SetReload((uint8_t)254);//RELOAD_VALUE=254  1.724s
   IWDG_ReloadCounter();
}


int main(void)
{
        u8 i;
	clock_init();
        iwdg_init();
	get_id_information();
	io_init();
//        testio_init();
	randtime_init();
	_1600_io_init();
	spi_init();
        se2438t_init();
//	_1600_registers_init(1);
	pvd_config();
        ble_proto_init();
        finder_init();
        timer4_cfg();
        
  	while(1)
 	{
          
            _CE__0;//WL1600失能
            delay_ms(2);
            _CE__1;//WL1600使能
            delay_ms(1);
            _1600_registers_init(0);//初始化WL1600寄存器，配置为BLE模式
            
//           if(finder_is_oper_timeout == 1){
//              finder_is_oper_timeout = 0;
//              finder_proc_oper_timeout();
//            }
//          
            for(i=0;i<loop_count;i++){//BLE 发送和接收
              IWDG_ReloadCounter();

                 ble_proto_run();
              delay_ms(100);
            }
          
          
            IWDG_ReloadCounter();

             _CE__0;//WL1600失能
             delay_ms(2);
            _CE__1;//WL1600使能
  //           delay_ms(1);
            _1600_registers_init(1);//初始化WL1600寄存器，配置为 防盗器 模式

            if(dActiveFlag)
            {
  //            //timecnt=0;
              output_message();
            
            }
            else
            {
              detect_process();
            }
            
             _CE__0;//WL1600失能
             sleep(t_space);//MCU休眠
            after_wakeup();
//            delay_ms(90);
            _CE__1;//WL1600使能
            delay_ms(1);
     
  	}
  	return 0;
}

static u8 dTimeCnt=0;
extern u8 RXERROR;
extern u8 TXERROR;
extern u8 CANNOTSLEEP;
extern u8 CANNOTRESET;
extern u8 SPIERROR;

static void abnormal_cheke_process(void)
{
    u8 d1600test=0;
    /*if((read1600(70)&0x40)) //delete
    {
      WWDG_SWReset();//resume
    }*/
    //异常事件计数写入eeprom
    //tx rx 标志位判断 如果置起，复位
    
    if((read1600(15)&0x80))
    {
      save_error_msg(RXERROR);
      WWDG_SWReset();
    }
    if((read1600(14)&0x01))
    {
      save_error_msg(TXERROR);
      WWDG_SWReset();
    }
    if((read1600(21)&0x01)==0)
    {
      save_error_msg(CANNOTSLEEP);
      WWDG_SWReset();
    }
    d1600test=read1600(31);
    if((d1600test&0x03)==3)
    {
      save_error_msg(CANNOTRESET);
      write1600(31,d1600test&0xFC);
    }
    if(dTimeCnt++>=100)
    {
      dTimeCnt=0;
      if(read1600(1)!=0xE9)
      {
        save_error_msg(SPIERROR);
        WWDG_SWReset();
      }
    }
}

static void detect_process(void)
{
  u8 i,r_length=0;
  u16 r_info=0;
  _1600_clean_rx_fifo();
  write1600(15,receive_channel);
  se2438t_enter_high_gain_R_mode();
  _1600_rx_on();
//  TESTIO_TOGGLE;
  for(i=0;i<22;i++)
  {
    if(_1600_pkt_status()==1)
    {
      if(_1600_crc_verify_result()==0)
      {
        r_length=read1600(100);
        if(r_length==2)
        {
          r_info=(read1600(100)<<8);
          r_info += read1600(100);
          t_info = (r_info&0x8000)>>15;
          t_pa = (r_info&0x7000)>>12;
          t_times= send_times[(r_info&0x0F00)>>8];
          t_space= send_space[(r_info&0x00F0)>>4];
          t_para_test=(r_info&0x000F);
          if(t_pa<=5)
          {
            write1600(18,send_pa[t_pa]);
            write1600(15,send_channel);
            dActiveFlag = TRUE;
          }
        }
        else
        {
//          t_pa = 5;
//          t_times= 6;
//          t_space= 3;
//          write1600(18,send_pa[t_pa]);
//          write1600(15,send_channel);
          dActiveFlag = TRUE;
        }
        break;
      }
      else
      {
//        t_pa = 5;
//        t_times= 6;
//        t_space= 3;
//        write1600(18,send_pa[t_pa]);
//        write1600(15,send_channel);
        dActiveFlag = TRUE;
        break;
      }
    }
  }
//  TESTIO_TOGGLE;
//  write1600_bit(70,6,1);
   DISABLE_RX();
  se2438t_enter_all_off_mode();
  if(!dActiveFlag)
  {
//    t_space=3;
//    sleep(t_space);
//    after_wakeup();
  }
}

static void output_message(void)
{
  s8 i=0;
  u16 cnt=0;
  write1600(15,send_channel);

  _1600_clean_tx_fifo();
//  if(t_para_test==0x05&&t_info)
//  {
//    ADC_Config();
//    get_voltage();
//    write1600(100,19);
//    write1600(100,read1600(18));
//    write1600(100,t_space);
//    write1600(100,t_times);
//    if(totaltimes>=0xFF)
//    {
//      totaltimes=0;
//    }
//    for(i=0;i<8;i++)
//    {
//      write1600(100,dOwnInfoBuf[i]);
//    }
//    for(i=0;i<=5;i++)
//    {
//      write1600(100,dCustInfoBuf[i]);
//    }
//    write1600(100,((PotVoltage/100)&0xFF));
//    write1600(100,((PotVoltage%100)&0xFF));
//  }
//  else
//  {
    write1600(100,9);
    write1600(100,read1600(18));
    write1600(100,t_space);
    for(i=0;i<=5;i++)
    {
      write1600(100,dCustInfoBuf[i]);
    }
    write1600(100,dVDDlow);
//  }
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
  DISABLE_TX();
  se2438t_enter_all_off_mode();

  write1600_bit(70,6,1);
  if((--t_times)<=0)
  {
    dActiveFlag=FALSE;
  }
//  sleep(t_space);
//  after_wakeup();
}

