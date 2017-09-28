#ifndef _1600_h_
#define _1600_h_

#include "stm8l15x.h"
#include "spi.h"
#include "delay.h"
#include "wdg.h"

#define IO_CE_SRC	GPIOB
#define IO_CE_PIN	GPIO_Pin_3
#define IO_CS_SRC	GPIOB
#define IO_CS_PIN	GPIO_Pin_4
#define IO_PKT_SRC	GPIOC
#define IO_PKT_PIN	GPIO_Pin_0

#define _CE__1	IO_CE_SRC->ODR|=IO_CE_PIN
#define _CE__0	IO_CE_SRC->ODR&=~IO_CE_PIN
#define _CS__1	IO_CS_SRC->ODR|=IO_CS_PIN
#define _CS__0	IO_CS_SRC->ODR&=~IO_CS_PIN
#define _CS__TOGGLE     IO_CS_SRC->ODR^=IO_CS_PIN
#define _PKT_LV__	(GPIOC->IDR&GPIO_Pin_0)


#define RF_CHN_TO_PLL_CHN(_x_)  ((_x_) * 2) 

/*
 * Macros to control WL1600.
 * Macros name their functions.
 */
#define ENABLE_BLE_SYNC_WORD() \
    write1600(73, 0xD6)

#define DISABLE_BLE_SYNC_WORD() \
    write1600(73, 0x00)

/* ¦Ì¡Â??VCO2???¦Ì?¨¢¡Â */
#define WRITE_TX_VCO() \
    write1600(52, 0x1A)

/* ¦Ì¡Â??VCO2???¦Ì?¨¢¡Â */
#define WRITE_RX_VCO() \
    write1600(52, 0x1E)

#define ENABLE_PACK_LEN() \
    write1600(82, 0x31)

#define DISABLE_PACK_LEN() \
    write1600(82, 0x11)

#define ENABLE_PKT_FLAG_INTR() \
    write1600(33, 0x02)

#if (USE_SH77 == 1)
#define POLL_PKT_FLAG_INTR_STAT() \
    WL1600_PKT_FLAG_PIN
#else
#define POLL_PKT_FLAG_INTR_STAT() \
    GPIO_ReadInputDataBit(GPIOC, GPIO_Pin_0)
#endif

/* Currently no effect */
#define ACK_PKT_FLAG_INTR_STAT() \
    write1600(37, 0x02)

#define RESET_FIFO_WR_PTR() \
    write1600(104, 0x80)

#define READ_FIFO_WR_PTR() \
    read1600(104)

#define RESET_FIFO_RD_PTR() \
    write1600(105, 0x80)

#define READ_FIFO_RD_PTR() \
    read1600(104)

#define READ_FIFO() \
    read1600(100)

#define READ_FIFO_LEN(_buf_, _len_) \
    wl1600_read_fifo(100, _buf_, _len_)

#define WRITE_FIFO(_x_) \
    write1600(100, _x_)

#define WRITE_FIFO_LEN(_buf_, _len_) \
    wl1600_write_fifo(100, _buf_, _len_)

//#define ENABLE_RX() \
//    _1600_rx_on()//write1600_bit(15, 7, 1)

#define DISABLE_RX() \
    write1600_bit(15, 7, 0)

//#define ENABLE_TX() \
//    _1600_tx_on()//write1600_bit(14, 0, 1)

#define DISABLE_TX() \
    write1600_bit(14, 0, 0)

#define SET_1600_SLEEP_MODE() \
    write1600_bit(70, 6, 1)
    
#define SET_1600_IDLE_MODE() \
    write1600_bit(70, 6, 0)

void _1600_io_init(void);
void wakeup_1600(void);
void _1600_registers_init(u8 mode);

volatile u8 read1600(u8 reg);
void write1600(u8 reg,u8 value);
void write1600_bit(u8 reg,u8 bit,u8 value);
u8 read1600_bit(u8 reg,u8 bit);

void wl1600_read_fifo(u8 reg, u8 *buf, u8 len);
void wl1600_write_fifo(u8 reg, u8 *buf, u8 len);

void _1600_clean_rx_fifo(void);
void _1600_rx_on(void);
u8 _1600_pkt_status(void);
void _1600_clear_pkt(void);
u8 _1600_crc_verify_result(void);
void _1600_clean_tx_fifo(void);
void _1600_tx_on(void);
void _1600_frequency_channel(u8 value);

void switch_mode(u8 mode);
#endif
