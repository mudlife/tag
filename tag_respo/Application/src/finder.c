

#include "delay.h"
#if (USE_SH77 == 1) 
#include "exti.h"
#endif
#include "1600.h"
#include "ble_pkts.h"
#include "ble_proto.h"
#include "finder.h"
#include <string.h>
#include "testio.h"

extern u8 loop_count;
extern u8 t_space;


/** Number of the same command received before processing */
#define MIN_CMD_CNT         (2)
#if (USE_SH77 == 1) 
extern void os_clk_4M_init();
extern void os_clk_32k_init();
#define SET_OS_4M_CLOCK()   os_clk_4M_init()
#define SET_OS_32K_CLOCK()  os_clk_32k_init()
#endif
/** iBeacon prefix bytes before the UUID */
const u8  ibeacon_prefix[] = {
    0x02, 0x01, 0x06,
    0x1A, 0xFF,
    0x4C, 0x00, // Company ID
    0x02,
    0x15
};
/** Special company ID for pairing iBeacon */
#define COMPANY_ID_FOR_PAIRING      (0xFFFF)

/** Finder's UUID. The last 6 bytes could be the BD_ADDR */
const u8  uuid1600[]={
    0xBE, 0xAC, 0x16, 0x00,
    0x00, 0x00,
    0x00, 0x00,
    0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00
};

u8  rst[6]={0xe4, 0xc0, 0xe0, 0xc0, 0xe0, 0x22};    //软件复位功能，定义一个code类型的数组，一定要为code类型
#define SOFT_RESET()     (*((void (*)())rst))()
/** Tx power in iBeacon */
#define TX_POWER                    (0xC8)

/** Finder's current state */
u8 curr_state;
/** Finder's current command sequence number */
u8 curr_seq;
/** The command extracted from the received iBeacon */
u8 rx_cmd;
/** The command sequence number extracted from the received iBeacon */
u8 rx_cmd_seq;
/** Number of the same command received */
u8 rx_cmd_cnt;

//u8 user_action = BTN_ACTION_NONE;

/** Countdown of current operation */
u8 finder_oper_cntdn; // in 250ms
/** Indicate timeout(1) or not(0) */
u8 finder_is_oper_timeout;
/** Next operation upon timeout */
u8 finder_oper_next;

u8 finder_start_flag = FINDER_START_MODE;
/** Set timer and execute the specified operation upon timeout */
/* TODO: put next operation to queue if finder_oper_cntdn != 0 */
#define FINDER_OPER_SET_TIMER(_cntdn_sec_, _next_oper_) \
    do { \
        finder_oper_cntdn = (_cntdn_sec_ * 1000 / FINDER_OPER_TIME_BASE); \
        finder_oper_next = _next_oper_; \
    } while (0)


/**
 * Generate beacon packet.
 *
 * @param cmd The command to be packed in iBeacon packet.
 * @param state The state to be packed in iBeacon packet.
 */
void finder_gen_beacon(u8 cmd, u8 state)
{
    IBEACON finder_beacon;

    memcpy(finder_beacon.prefix, ibeacon_prefix, sizeof(ibeacon_prefix));
    memcpy(finder_beacon.uuid, uuid1600, sizeof(uuid1600));
    if (state == FINDER_STAT_PAIRING) {
        /* 
         * Use the company ID other than Apple's so that iOS CoreBluetooth
         * won't block this beacon.
         */
        finder_beacon.prefix[PREFIX_CID_IDX] = COMPANY_ID_FOR_PAIRING >> 8;
        finder_beacon.prefix[PREFIX_CID_IDX + 1] = COMPANY_ID_FOR_PAIRING & 0xFF;
    }
    /*
     * Expose our MAC address in UUID since iOS CoreBluetooth prevents the
     * ADVA from being read by App. Besides, App can only monitor iBeacon
     * by register specific UUID to iOS CoreLocation.
     */
    memcpy(&finder_beacon.uuid[UUID_FINDER_MAC_IDX], adv_address, DEV_ADDR_LEN);

    finder_beacon.major[1] = cmd;
    if (cmd != FINDER_CMD_NONE) {
        finder_beacon.major[1] |= (++curr_seq << 4);
    }
    finder_beacon.minor[1] = state;
    
    finder_beacon.major[0] = ~finder_beacon.major[1];
    finder_beacon.minor[0] = ~finder_beacon.minor[1];
    finder_beacon.tx_power = TX_POWER;

    ble_gen_adv_ind_pkts((u8 *)&finder_beacon, sizeof(IBEACON));
}

/**
 * Process operation timeout.
 */
void finder_proc_oper_timeout(void)
{
    if(curr_state == FINDER_STAT_ALERT){
      loop_count = 2;
      t_space = 10;
      finder_set_oper(FINDER_OPER_ADV);
    }else{
      switch (finder_oper_next) {
      case FINDER_OPER_NEXT_IDLE:
          curr_state = FINDER_STAT_IDLE;
      case FINDER_OPER_NEXT_CURR_STATE:
          finder_set_oper(FINDER_OPER_ADV);
          break;
      }
    }
    finder_oper_next = FINDER_OPER_NEXT_NONE;
  
}

/**
 * Set finder's operation.
 *
 * @param oper The operation to be set. See enum _FINDER_OPER.
 */
void finder_set_oper(u8 oper)
{
    switch (oper) {
    case FINDER_OPER_ADV:
       /* Advertise according to current state */
        finder_gen_beacon(FINDER_CMD_ADV_TIME_3, curr_state);
        switch (curr_state) {
        case FINDER_STAT_PAIRING:
//            ble_start_adv_timer(500, 100);
            /* Set timer */
//            FINDER_OPER_SET_TIMER(300, FINDER_OPER_NEXT_IDLE);
            break;
        case FINDER_STAT_IDLE:
//            ble_start_adv_timer(1000, 300);
            break;
        case FINDER_STAT_ALERT:
          FINDER_OPER_SET_TIMER(20, FINDER_OPER_NEXT_IDLE);
            //finder_is_oper_timeout = 0;
//            ble_start_adv_timer(500, 100);
            break;
	case FINDER_STAT_SEARCH_ON:
//            ble_start_adv_timer(500, 100);
//            /* Set timer */
            FINDER_OPER_SET_TIMER(2, FINDER_OPER_NEXT_IDLE);
            break;
	case FINDER_STAT_SEARCH_OFF:
//            ble_start_adv_timer(500, 100);
//            /* Set timer */
            FINDER_OPER_SET_TIMER(2, FINDER_OPER_NEXT_IDLE);
            break;
        }
        ble_set_state(BLE_STATE_ADV);
        break;
    case FINDER_OPER_SCAN:
        ble_start_adv_timer(1000, 300);
        ble_set_state(BLE_STATE_SCAN);
        /* Set timer */
        FINDER_OPER_SET_TIMER(1, FINDER_OPER_NEXT_CURR_STATE);
        break;
   
    }
}

/**
 * Process command.
 *
 * @param cmd The command to be processed.
 *
 * @return 0 if PDU is invalid or command makes no BLE protocol change.
 *         1 if command makes BLE protocol change.
 */ 

u8 finder_proc_cmd(u8 cmd)
{
    switch (cmd) {
    case FINDER_CMD_ENABLE_ALERT://?a??·à?a
//        if (curr_state == FINDER_STAT_PAIRING) {
//            /* Prompt for pairing completion */
////            beep_start(10, 5, 3);
//        }
        if (curr_state != FINDER_STAT_ALERT) {
            curr_state = FINDER_STAT_ALERT;
            finder_set_oper(FINDER_OPER_ADV);
//            TESTIO_TOGGLE;
//          delay_ms(10);
//          TESTIO_TOGGLE;
           return (1);
        }
        break;
    case FINDER_CMD_DISABLE_ALERT://1?±?·à?a
//        if (curr_state == FINDER_STAT_ALERT) {
//            curr_state = FINDER_STAT_IDLE;
            finder_set_oper(FINDER_OPER_ADV);
//            return (1);
//        }
        break;
    case FINDER_CMD_PAIRING_REQ://???????ó

//	if (curr_state != FINDER_STAT_IDLE) {
//						beep_start(50, 50, 1);
            curr_state = FINDER_STAT_IDLE;
            finder_set_oper(FINDER_OPER_ADV);
           
//            return (1);
//        }
        break;
    case FINDER_CMD_SEARCH_ON://?°?ò?a

        curr_state = FINDER_STAT_SEARCH_ON;
        finder_set_oper(FINDER_OPER_ADV);

        return (1);

        break;
    case FINDER_CMD_SEARCH_OFF://?°?ò1?

        curr_state = FINDER_STAT_SEARCH_OFF;
        finder_set_oper(FINDER_OPER_ADV);

        return (1);

        break;
    default:
        break;
    }
    
    return (0);
}

/**
 * Process packet.
 *
 * @param pdu Pointer to an iBeacon PDU without ADVA.
 *
 * @return 0 if PDU is invalid or command makes no BLE protocol change.
 *         1 if command makes BLE protocol change.
 */
u8 finder_proc_pkt(u8 *pdu)
{
    IBEACON *ibeacon = (IBEACON *)pdu;
    u8 tmp_seq;
    u8 tmp_cmd;
    
    if (memcmp(&ibeacon->uuid[UUID_FINDER_MAC_IDX], adv_address,
            sizeof(adv_address)) != 0) {
        /* Invalid address identifier in UUID */
        return (0);
    }
    
    if (ibeacon->major[0] != (u8)~ibeacon->major[1]) {
        /* Verification on major fails */
        return (0);
    }

    /* Extract sequential number and command */
    tmp_seq = ibeacon->major[1] >> 4;
    tmp_cmd = ibeacon->major[1] & 0x0F;

    if ((tmp_seq != rx_cmd_seq) || (tmp_cmd != rx_cmd)) {
        /* New command. Reset command counter */
        rx_cmd_cnt = 1;
        rx_cmd_seq = tmp_seq;
        rx_cmd = tmp_cmd;

        /* Got one packet for us. Try to scan for more */
        finder_set_oper(FINDER_OPER_SCAN);

        return (1);
    }

    if ((rx_cmd_cnt != 0xFF) && (++rx_cmd_cnt == MIN_CMD_CNT)) {
        rx_cmd_cnt = 0xFF;
        /* Got the same packet N times. Process command */
       
        return finder_proc_cmd(tmp_cmd);
    }
    
    return (0);
}

/**
 * Check user's button action.
 */
//void finder_check_action(void)
//{
//#if (USE_SH77 == 1)     
//       /* Check user's action */
//    if (user_action != BTN_ACTION_NONE) {
//        if(finder_start_flag == FINDER_START_MODE) {
//            if (user_action == BTN_ACTION_CLICK) {           //单击就是拍照定位功能
//                 if(curr_state != FINDER_STAT_PAIRING){
//                    finder_set_oper(FINDER_OPER_PHOTO);
//                  }
//            } else if (user_action == BTN_ACTION_LONG1) {     //长按3s，配对功能
//                beep_start(10, 5, 2);
//            } else if (user_action == BTN_ACTION_LONG1_UP) {
//                 curr_state = FINDER_STAT_PAIRING;
//                 finder_set_oper(FINDER_OPER_ADV);
//            } else if (user_action == BTN_ACTION_LONG2) {
//                 beep_start(10, 5, 2);
//            } else if(user_action == BTN_ACTION_LONG2_UP) {
//                /* 运行中接收到 --> 关机*/   
//                finder_start_flag = FINDER_STOP_MODE;                    
//                ENABLE_EXIT();           //使能外部中断
//                SET_1600_SLEEP_MODE();    //1600进入睡眠模式
//              //  SET_OS_32K_CLOCK();
//                POWER_DOWN();            //MCU进入掉电模式,等待唤醒 
//              //  SET_OS_4M_CLOCK();
//                DISABLE_EXIT();          //MCU退出掉电模式后，关外部中断
//            }
//        user_action = BTN_ACTION_NONE;
//        btn_reset_action();
//        } else {
//             /** 接收到外部中断 --> 开机
//              * 如果用户是BTN_ACTION_CLICK、BTN_ACTION_LONG1 则继续进入掉电模式，等待唤醒
//              * 如果接收到开机命令，则软件复位 */
//            if((user_action == BTN_ACTION_CLICK) || (user_action == BTN_ACTION_LONG1_UP)) {
//                ENABLE_EXIT();           //使能外部中断
//            //    SET_OS_32K_CLOCK();
//                POWER_DOWN();            //MCU进入掉电模式 
//             //   SET_OS_4M_CLOCK();  
//                DISABLE_EXIT();          //MCU退出掉电模式后，关外部中断      
//                        
//            } else if (user_action == BTN_ACTION_LONG2) {
//                 beep_start(10, 5, 2);
//            } else if (user_action == BTN_ACTION_LONG2_UP) {
//                finder_start_flag = FINDER_START_MODE;                    
//                (*((void (*)())rst))();    //软件复位指令          
//            }
//            user_action = BTN_ACTION_NONE;
//            btn_reset_action();
//        
//        }
//    }
//#else
//    /* Check user's action */
////    if (user_action != BTN_ACTION_NONE) {
////        if (user_action == BTN_ACTION_CLICK) {
////            if(curr_state != FINDER_STAT_PAIRING){
////                finder_set_oper(FINDER_OPER_PHOTO);
////            }
////        } else if (user_action == BTN_ACTION_LONG1) {
////            beep_start(10, 5, 2);
////        } else if (user_action == BTN_ACTION_LONG1_UP) {
////            curr_state = FINDER_STAT_PAIRING;
////            finder_set_oper(FINDER_OPER_ADV);
////        }
////        user_action = BTN_ACTION_NONE;
////        btn_reset_action();
////    }
//#endif /* #if (USE_SH77 == 1) */
//
//}

/**
 * Initialize finder.
 */
void finder_init(void)
{
    curr_state = FINDER_STAT_PAIRING;
    curr_seq = 0;
    finder_oper_cntdn = 0;   
//    finder_is_oper_timeout = 0;
    
    rx_cmd = FINDER_CMD_NONE;
    rx_cmd_seq = 0xFF;
    rx_cmd_cnt = 0;
    
    finder_set_oper(FINDER_OPER_ADV);
    FINDER_OPER_SET_TIMER(300, FINDER_OPER_NEXT_IDLE);
    
}
