

#include "1600.h" 
#include "ble_pkts.h"
#include "ble_proto.h"
#include "finder.h"
#include "timer.h"
#include "spi.h"
#include "se2438t.h"

/*
 *           adv interval            adv delay           adv interval            adv delay
 * <-------------------------------><---------><-------------------------------><--------->
 * <----->                                     <----->
 *  chn interval                                chn interval
 *        <----->                                     <----->
 *         chn interval                                chn interval
 *               <----->                                     <----->
 *                chn interval                                chn interval
 */
u16 adv_interval_reload = 0; // ms
u16 adv_interval_cntdn = 0;
volatile u8 is_adv_interval_started = 0;
#define ADV_DELAY               (10) // ms. Core Spec: range from 0~10ms
u16 adv_delay_cntdn = 0;
u16 chn_interval_reload = 0; // ms
u16 chn_interval_cntdn = 0;
volatile u8 is_chn_interval_started = 0;

/* Channels used for advertising */
const u8  adv_chns[] = {37, 38, 39};
u8 adv_chn_idx = 0;

/* All BLE channels mapped to PLL ones*/
const u8  ble_channels[] = {
    RF_CHN_TO_PLL_CHN(1),   // CH#0
    RF_CHN_TO_PLL_CHN(2),   // CH#1
    RF_CHN_TO_PLL_CHN(3),   // CH#2
    RF_CHN_TO_PLL_CHN(4),   // CH#3
    RF_CHN_TO_PLL_CHN(5),   // CH#4
    RF_CHN_TO_PLL_CHN(6),   // CH#5
    RF_CHN_TO_PLL_CHN(7),   // CH#6
    RF_CHN_TO_PLL_CHN(8),   // CH#7
    RF_CHN_TO_PLL_CHN(9),   // CH#8
    RF_CHN_TO_PLL_CHN(10),  // CH#9
    RF_CHN_TO_PLL_CHN(11),  // CH#10
    RF_CHN_TO_PLL_CHN(13),  // CH#11
    RF_CHN_TO_PLL_CHN(14),  // CH#12
    RF_CHN_TO_PLL_CHN(15),  // CH#13
    RF_CHN_TO_PLL_CHN(16),  // CH#14
    RF_CHN_TO_PLL_CHN(17),  // CH#15
    RF_CHN_TO_PLL_CHN(18),  // CH#16
    RF_CHN_TO_PLL_CHN(19),  // CH#17
    RF_CHN_TO_PLL_CHN(20),  // CH#18
    RF_CHN_TO_PLL_CHN(21),  // CH#19
    RF_CHN_TO_PLL_CHN(22),  // CH#20
    RF_CHN_TO_PLL_CHN(23),  // CH#21
    RF_CHN_TO_PLL_CHN(24),  // CH#22
    RF_CHN_TO_PLL_CHN(25),  // CH#23
    RF_CHN_TO_PLL_CHN(26),  // CH#24
    RF_CHN_TO_PLL_CHN(27),  // CH#25
    RF_CHN_TO_PLL_CHN(28),  // CH#26
    RF_CHN_TO_PLL_CHN(29),  // CH#27
    RF_CHN_TO_PLL_CHN(30),  // CH#28
    RF_CHN_TO_PLL_CHN(31),  // CH#29
    RF_CHN_TO_PLL_CHN(32),  // CH#30
    RF_CHN_TO_PLL_CHN(33),  // CH#31
    RF_CHN_TO_PLL_CHN(34),  // CH#32
    RF_CHN_TO_PLL_CHN(35),  // CH#33
    RF_CHN_TO_PLL_CHN(36),  // CH#34
    RF_CHN_TO_PLL_CHN(37),  // CH#35
    RF_CHN_TO_PLL_CHN(38),  // CH#36
    RF_CHN_TO_PLL_CHN(0),   // CH#37
    RF_CHN_TO_PLL_CHN(12),  // CH#38
    RF_CHN_TO_PLL_CHN(39),  // CH#39
};

/* BLE protocol state */
u8 ble_curr_state = BLE_STATE_ADV;
u8 ble_next_state = BLE_STATE_ADV;


/**
 * Manage advertising interval.
 * This function is called every 1ms to manage advertising interval.
 */
void ble_manage_adv_interval(void)
{
    /*
     * Note: "adv_interval_cntdn > 1" -- when timeout occurs, 1 ms has passed.
     * Using this statement is just for accuracy.
     */
    if (adv_interval_cntdn > 1) {
        adv_interval_cntdn--;
        if (--chn_interval_cntdn == 0) {
            chn_interval_cntdn = chn_interval_reload;
            is_chn_interval_started = 1;
        }
    } else {

        if (adv_delay_cntdn-- == 0) {
            adv_delay_cntdn = ADV_DELAY; // TODO: random ADV delay
            adv_interval_cntdn = adv_interval_reload;
            chn_interval_cntdn = chn_interval_reload;
            is_adv_interval_started = 1;
            is_chn_interval_started = 1;

        }
    }
}

/**
 * Start advertising event timer.
 *
 * @param adv_interval The advertising interval(ms) in an advertising event.
 * @param chn_interval The time to stay in a channel.
 */
void ble_start_adv_timer(u16 adv_interval, u16 chn_interval)
{
    DISABLE_BLE_TIMER();
    adv_interval_cntdn = adv_interval_reload = adv_interval;
    chn_interval_cntdn = chn_interval_reload = chn_interval;
    adv_delay_cntdn = ADV_DELAY;
    adv_chn_idx = 0;
    is_chn_interval_started = 1;
//    RELOAD_BLE_TIMER();
    ENABLE_BLE_TIMER();
}

/**
 * Set BLE channel.
 *
 * @param channel The BLE channel to set.
 */
void ble_set_channel(u8 channel)
{    
    if (channel <= sizeof(ble_channels)) {
        _1600_frequency_channel(ble_channels[channel]);
    }
}

/**
 * Transmit a BLE packet.
 *
 * @param buf The pointer to a buffer of data to be sent.
 * @param len The length of data to be sent.
 */
void ble_tx(u8 *buf, u8 len)
{
    DISABLE_BLE_SYNC_WORD();
    WRITE_TX_VCO();
    ENABLE_PACK_LEN();
    ENABLE_PKT_FLAG_INTR();
    RESET_FIFO_WR_PTR();
    _1600_clean_tx_fifo();
    /* Write packet length */
    WRITE_FIFO(len);
    /* Write data bytes */
    WRITE_FIFO_LEN(buf, len);
    se2438t_enter_T_mode();
    _1600_tx_on();
#if (USE_SH77 == 1) 	
    delay_10us();
#else
    delay_us(3);
#endif
    
#if (USE_SH77 == 1) 
    /* Wait for PKT_FLAG signal */
	while (WL1600_PKT_FLAG_PIN == 0);
#else
    /* Wait for PKT_FLAG signal */
	while (POLL_PKT_FLAG_INTR_STAT() == 0);
#endif
        
        se2438t_enter_all_off_mode();
}

/**
 * Receive a BLE packet.
 *
 * @param buf The pointer to a buffer to store received data.
 * @param len The length we expected to receive.
 *
 * @return 0 if Rx timed out.
 *         1 if shorter packet is received.
 *         exp_len if packet of expected length is received.
 */
u8 ble_rx(u8 *buf, u8 exp_len)
{
#define IS_RX_TIMEOUT       is_chn_interval_started

    u8 i;
    u8 retry;

    ENABLE_BLE_SYNC_WORD();
    WRITE_RX_VCO();
    DISABLE_PACK_LEN();
    RESET_FIFO_WR_PTR();
    RESET_FIFO_RD_PTR();
    _1600_clean_rx_fifo();
    se2438t_enter_high_gain_R_mode();
    _1600_rx_on();
#if (USE_SH77 == 1) 	
    delay_10us();
#else
    delay_us(3);
#endif

    while (1) {
        if (is_chn_interval_started == 1) {
            is_chn_interval_started = 0;
            DISABLE_RX();
            return (0);
        }
        if (POLL_PKT_FLAG_INTR_STAT() == 1) {
            break;
        }
    }

#if (USE_SH77 == 0)
    /*
     * Wait for data of expected length
     *
     * Add retry to prevent from a WL1600 bug that FIFO write pointer is always 1.
     */
    retry = exp_len * 2;
    while ((i = READ_FIFO_WR_PTR()) < exp_len) {
        if (--retry == 0) {
            DISABLE_RX();
            return (1);
        }
    }
#endif

    /* We have received enough data. Stop Rx */
    DISABLE_RX();

    /* Get data */

    READ_FIFO_LEN(buf, exp_len);

    /* Dewhitening */
    ble_fast_dewhiten(buf, exp_len, 1, adv_chns[adv_chn_idx]);

#if 0
    for (i = 0; i < exp_len; ++i) {
        swo_printf("%02X ", buf[i]);
    }
    swo_printf("\r\n");
#endif
    
    se2438t_enter_all_off_mode();
    return (exp_len);
}

/**
 * Set the next state in BLE protocol.
 */
void ble_set_state(u8 state)
{
    ble_next_state = state;
}

/**
 * Initialize BLE protocol.
 */
void ble_proto_init(void)
{
    ble_pkts_init();
//    ble_timer_init();
}

/**
 * Run BLE protocol.
 */
void ble_proto_run(void)
{
     BLE_PKT tmp;
    u8 rx_len;
    u8 rx_index = 2;
    /* Update state */
    ble_curr_state = ble_next_state;
    
    if (is_chn_interval_started == 0) {    //如果chn_interval没有到，就等待，什么都不做
//        SET_1600_SLEEP_MODE();
//        POWER_IDLE();          //mcu进入空闲模式(2.8mA)
//        SET_1600_IDLE_MODE();
//        
        if (is_chn_interval_started == 0) {    //如果chn_interval没有到，就等待，什么都不做
            /* Channel interval not started yet */
            return;
        }
    }
    is_chn_interval_started = 0;           //如果到了就设为0，并进行下面的处理

    if (is_adv_interval_started == 1) {        //如果是新的adv interval开始了，就从37信道开始，否则到下一个信道
        is_adv_interval_started = 0;
        /* New advertising interval has started. Reset channel index */
        adv_chn_idx = 0;
    } else {
        adv_chn_idx++;
    }

    if (adv_chn_idx >= sizeof(adv_chns)) {
        /* All channels have been used in current advertising interval */
        return;
    }



    /* Change channel */
    ble_set_channel(adv_chns[adv_chn_idx]);

    if (ble_curr_state == BLE_STATE_ADV) {
        /* Send ADV packet */
      
        ble_send_adv_ind_pkt(adv_chns[adv_chn_idx]);
       
    }
    
#define IBEACON_PDU_LEN     (6 + 30) // 6-byte ADVA, 30-byte IBEACON
#define EXP_LEN             (1 + IBEACON_PDU_LEN) // 1-byte Length

    do {
        /* Try to receive iBeacon */
        
        rx_len = ble_rx(&tmp.pdu.adv_ind.header.bytes[1], EXP_LEN);
      
        if ((rx_len >= EXP_LEN) &&
                (tmp.pdu.adv_ind.header.bits.length == IBEACON_PDU_LEN)) {
            /* Probably iBeacon to our device */
            if (finder_proc_pkt(tmp.pdu.adv_ind.advd)) {
                /* BLE protocol/timer changed by command */
                break;
            }
        } else if (rx_len == 0) {
            /* Rx timeout */
            break;
        }
    } while (ble_curr_state == BLE_STATE_SCAN || rx_index--);

    
}
