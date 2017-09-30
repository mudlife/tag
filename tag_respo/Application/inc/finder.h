#ifndef __FINDER_H
#define __FINDER_H

#include "stm8l15x.h"

/* Finder's commands */
#define FINDER_CMD_NONE             (0x0)
#define FINDER_CMD_ENABLE_ALERT     (0x1)
#define FINDER_CMD_DISABLE_ALERT    (0x2)
#define FINDER_CMD_PAIRING_REQ       (0x3)
#define FINDER_CMD_SEARCH_ON        (0x4)
#define FINDER_CMD_PHOTO            (0x5)
#define FINDER_CMD_SEARCH_OFF       (0x6)

#define FINDER_CMD_ADV_TIME_3       (0xA)
/* Finder's states */
#define FINDER_STAT_NONE            (0x0)
#define FINDER_STAT_PAIRING         (0x1)
#define FINDER_STAT_IDLE            (0x2)
#define FINDER_STAT_ALERT           (0x3)
#define FINDER_STAT_SEARCH_ON	    (0x4)
#define FINDER_STAT_SEARCH_OFF      (0x6)

#define FINDER_START_MODE           (1)
#define FINDER_STOP_MODE            (0)

/* Finder's operation */
enum _FINDER_OPER {
    FINDER_OPER_ADV = 1,
    FINDER_OPER_SCAN,
    FINDER_OPER_PHOTO
};

enum _FINDER_OPER_NEXT {
    FINDER_OPER_NEXT_NONE,
    FINDER_OPER_NEXT_IDLE,
    FINDER_OPER_NEXT_CURR_STATE
};

/* The position of company ID in prefix */
#define PREFIX_CID_IDX              (5)
/* The position of finder's MAC address in UUID */
#define UUID_FINDER_MAC_IDX         (10)

/* iBeacon format in ADV data */
typedef struct _IBEACON{
    u8 prefix[9];
    u8 uuid[16];
    u8 major[2];
    u8 minor[2];
    u8 tx_power;
} IBEACON;
 
/* The time base used for finder's operation countdown */
#define FINDER_OPER_TIME_BASE       (250) // ms

extern u8 finder_oper_cntdn;
extern u8 finder_is_oper_timeout;

extern void finder_proc_oper_timeout(void);
extern void finder_set_oper(u8 oper);
extern u8 finder_proc_pkt(u8 *pdu);
//extern void finder_check_action(void);
extern void finder_init(void);
 
#endif /* __FINDER_H */
