#ifndef __BLE_PKTS_H
#define __BLE_PKTS_H

#include "stm8l15x.h"


/* Single Bit definitions */
#ifndef BIT0
#define BIT0    (0x01)
#define BIT1    (0x02)
#define BIT2    (0x04)
#define BIT3    (0x08)
#define BIT4    (0x10)
#define BIT5    (0x20)
#define BIT6    (0x40)
#define BIT7    (0x80)
#endif

/*
 * Different length in a packet
 */
#define PREAMBLE_LEN            (1)
#define ACCESS_ADDRESS_LEN      (4)
#define PDU_HEADER_LEN          (2)
#define MAX_PDU_PAYLOAD_LEN     (37)
#define CRC_LEN                 (3)
#define MAX_DATA_WHITENING_LEN  (PDU_HEADER_LEN + MAX_PDU_PAYLOAD_LEN + CRC_LEN)
#define DEV_ADDR_LEN            (6)

/* PDU header of packets in ADV channels */
typedef struct _ADV_PDU_HEADER {
    u8 rx_add: 1;
    u8 tx_add: 1;
    u8 rfu0: 2;
    u8 type: 4;
    u8 length: 6;
    u8 rfu1: 2;
} ADV_PDU_HEADER;

/* PDU header fields */
#define PDU_HDR_TXADDR_RANDOM       BIT6
#define PDU_HDR_RXADDR_RANDOM       BIT7
#define PDU_HDR_TYPE_MASK           (0x0F)
#define PDU_HDR_TYPE_ADV_IND        (0)
#define PDU_HDR_TYPE_ADV_DIRECT_IND  (1)
#define PDU_HDR_TYPE_ADV_NONCONN_IND (2)
#define PDU_HDR_TYPE_SCAN_REQ       (3)
#define PDU_HDR_TYPE_SCAN_RSP       (4)
#define PDU_HDR_TYPE_CONN_REQ       (5)
#define PDU_HDR_TYPE_ADV_SCAN_IND   (6)
#define PDU_HDR_LENGTH_MASK         (0x3F)

/* ADV_IND PDU */
typedef struct _ADV_IND_PDU {
    union {
        ADV_PDU_HEADER bits;
        u8 bytes[2];
    } header;
    u8 adva[DEV_ADDR_LEN];
    u8 advd[31];
} ADV_IND_PDU;

/* SCAN_REQ PDU */
typedef struct _SCAN_REQ_PDU {
    union {
        ADV_PDU_HEADER bits;
        u8 bytes[2];
    } header;
    u8 scana[DEV_ADDR_LEN];
    u8 adva[DEV_ADDR_LEN];
} SCAN_REQ_PDU;

/* SCAN_RSP PDU */
typedef struct _SCAN_RSP_PDU {
    union {
        ADV_PDU_HEADER bits;
        u8 bytes[2];
    } header;
    u8 adva[DEV_ADDR_LEN];
    u8 rspd[31];
} SCAN_RSP_PDU;

/* CONN_REQ PDU */
typedef struct _CONN_REQ_PDU {
    union {
        ADV_PDU_HEADER bits;
        u8 bytes[2];
    } header;
    u8 inita[DEV_ADDR_LEN];
    u8 adva[DEV_ADDR_LEN];
    u8 lld[22];
} CONN_REQ_PDU;

/* An entire BLE packet plus certain attributes */
typedef struct _BLE_PKT {
    u8 preamble;
    u8 access_address[ACCESS_ADDRESS_LEN];
    union {
        ADV_IND_PDU adv_ind;
        SCAN_REQ_PDU scan_req;
        SCAN_RSP_PDU scan_rsp;
        CONN_REQ_PDU conn_req;
    } pdu;
    u8 crc[CRC_LEN];
    /* Packet length (from preamble to the end of crc). Not part of a real BLE packet */
    u8 len;
} BLE_PKT;


extern const u8  adv_address[DEV_ADDR_LEN];
extern BLE_PKT adv_ind_pkt;
extern void ble_pkts_init(void);
extern void ble_whiten(u8 *buf, u8 len, u8 channel);
extern void ble_fast_whiten(u8 *buf, u8 len, u8 offset, u8 channel);
#define ble_fast_dewhiten   ble_fast_whiten
extern void ble_send_adv_ind_pkt(u8 channel);
extern void ble_gen_adv_ind_pkts(u8 *advd, u8 advd_size);

#endif /* __BLE_PKTS_H */
