#ifndef __BLE_PROTO_H
#define __BLE_PROTO_H

#include "stm8l15x.h"

/* BLE states */
enum _BLE_STATE {
    BLE_STATE_ADV = 1,
    BLE_STATE_SCAN
};


extern u16 adv_interval_reload; 
extern u16 adv_interval_cntdn;
extern u16 chn_interval_reload; // ms
extern u16 chn_interval_cntdn;
extern u16 adv_delay_cntdn;
extern volatile u8 is_adv_interval_started;
extern volatile u8 is_chn_interval_started;

extern void ble_manage_adv_interval(void);
extern void ble_start_adv_timer(u16 adv_interval, u16 chn_interval);
extern void ble_tx(u8 *buf, u8 len);
extern u8 ble_rx(u8 *buf, u8 exp_len);
extern void ble_set_state(u8 state);
extern void ble_proto_init(void);
extern void ble_proto_run(void);

#endif /* __BLE_PROTO_H */
