
#include "ble_proto.h"
#include "finder.h"
#include "ble_pkts.h"
#include <string.h>

/* BLE packet sync for ADV packets */
const u8  adv_pkt_sync[] = {
    /* Preamble */
    0xAA,
    /* ADV Access Address */
    0xD6, 0xBE, 0x89, 0x8E
};

/* XXX: should be read from flash memory */
const u8  adv_address[DEV_ADDR_LEN] = {
    0xAA, 0xBB, 0xCC, 0x11, 0x22, 0x33
};

const u8  data_whitening_mask_chn37[MAX_DATA_WHITENING_LEN] = {
    0x8D, 0xD2, 0x57, 0xA1, 0x3D, 0xA7, 0x66, 0xB0,
    0x75, 0x31, 0x11, 0x48, 0x96, 0x77, 0xF8, 0xE3,
    0x46, 0xE9, 0xAB, 0xD0, 0x9E, 0x53, 0x33, 0xD8,
    0xBA, 0x98, 0x08, 0x24, 0xCB, 0x3B, 0xFC, 0x71,
    0xA3, 0xF4, 0x55, 0x68, 0xCF, 0xA9, 0x19, 0x6C,
    0x5D, 0x4C
};

const u8  data_whitening_mask_chn38[MAX_DATA_WHITENING_LEN] = {
    0xD6, 0xC5, 0x44, 0x20, 0x59, 0xDE, 0xE1, 0x8F,
    0x1B, 0xA5, 0xAF, 0x42, 0x7B, 0x4E, 0xCD, 0x60,
    0xEB, 0x62, 0x22, 0x90, 0x2C, 0xEF, 0xF0, 0xC7,
    0x8D, 0xD2, 0x57, 0xA1, 0x3D, 0xA7, 0x66, 0xB0,
    0x75, 0x31, 0x11, 0x48, 0x96, 0x77, 0xF8, 0xE3,
    0x46, 0xE9
};

const u8  data_whitening_mask_chn39[MAX_DATA_WHITENING_LEN] = {
    0x1F, 0x37, 0x4A, 0x5F, 0x85, 0xF6, 0x9C, 0x9A,
    0xC1, 0xD6, 0xC5, 0x44, 0x20, 0x59, 0xDE, 0xE1,
    0x8F, 0x1B, 0xA5, 0xAF, 0x42, 0x7B, 0x4E, 0xCD,
    0x60, 0xEB, 0x62, 0x22, 0x90, 0x2C, 0xEF, 0xF0,
    0xC7, 0x8D, 0xD2, 0x57, 0xA1, 0x3D, 0xA7, 0x66,
    0xB0, 0x75
};

const u8  *data_whitening_masks[] = {
    data_whitening_mask_chn37,
    data_whitening_mask_chn38,
    data_whitening_mask_chn39
};

 BLE_PKT adv_ind_pkt_chn37;
 BLE_PKT adv_ind_pkt_chn38;
 BLE_PKT adv_ind_pkt_chn39;
BLE_PKT  *adv_ind_pkts[] = {
    &adv_ind_pkt_chn37,
    &adv_ind_pkt_chn38,
    &adv_ind_pkt_chn39
};

/**
 * Swap bits between MSB and LSB of a given byte.
 *
 * @param byte The byte to be processed.
 *
 * @return The byte with swapped bits.
 */
u8 swap_bits(u8 byte)
{
    byte = (byte << 4) | (byte >> 4);
    byte = ((byte << 2) & 0xCC) | ((byte >> 2) & 0x33);
    byte = ((byte << 1) & 0xAA) | ((byte >> 1) & 0x55);

    return (byte);
}

/**
 * Swap bits for 3-byte CRC.
 *
 * @param crc The pointer to a 3-byte CRC that is to be bit-swapped.
 */
void swap_crc_bits(u8 *crc)
{
    u8 i;
    
    for (i = 0; i < 3; ++i) {
        *crc = swap_bits(*crc);
        crc++;
    }
}

/**
 * Calculate the CRC for ADV PDU.
 *
 * @param adv_pdu The pointer to the ADV PDU.
 * @param len The length of the ADV PDU.
 * @param dst The pointer to the 3-byte space for calculated CRC.
 */
void ble_calc_adv_crc(const u8 *adv_pdu, u8 len, u8 *crc)
{
    u8 v, t, d;

    /* Initial CRC value of ADV packets */
    crc[0] = 0x55;
    crc[1] = 0x55;
    crc[2] = 0x55;

    while (len--) {
        d = *adv_pdu++;
        for (v = 0; v < 8; v++, d >>= 1) {
            t = crc[0] >> 7;
            crc[0] <<= 1;
            if (crc[1] & 0x80) {
                crc[0] |= 1;
            }
            crc[1] <<= 1;
            if (crc[2] & 0x80) {
                crc[1] |= 1;
            }
            crc[2] <<= 1;
            if (t != (d & 1)) {
                crc[2] ^= 0x5B;
                crc[1] ^= 0x06;
            }
        }
    }

    swap_crc_bits(crc);
}

/**
 * Whiten the given data.
 *
 * @param buf The pointer to the data to be whitened.
 * @param len The length of the data.
 * @param channel The channel number for the whitening process.
 */
void ble_whiten(u8 *buf, u8 len, u8 channel)
{
    u8 m;
    u8 coeff = swap_bits(channel) | 0x02;
    
    while (len--) {
        for (m = 1; m; m <<= 1) {
            if (coeff & 0x80) {
                coeff ^= 0x11;
                *buf ^= m;
            }
            coeff <<= 1;
        }
        buf++;
    }
}

/**
 * Whiten the given data by using pre-whitened mask.
 *
 * @param buf The pointer to the data to be whitened.
 * @param len The length of the data.
 * @param offset The offset to the pre-whitened mask.
 * @param channel The channel number for the whitening process.
 */
void ble_fast_whiten(u8 *buf, u8 len, u8 offset, u8 channel)
{
    u8 i;
    const u8 *mask;

    if ((channel >= 37) && (channel <= 39)) {
        mask = data_whitening_masks[channel - 37];
        for(i = 0; i < len; i++) {
            buf[i] = buf[i] ^ mask[i + offset];
        }
    }       
}

/**
 * Compose ADV_IND packet, calculate CRC and whiten the data.
 */
void ble_gen_adv_ind_pkt(BLE_PKT *pkt, u8 *advd, u8 advd_size, u8 channel)
{
    u8 *crc_ptr;

    /* Fill the preamble and access address */
    memcpy(pkt, adv_pkt_sync, sizeof(adv_pkt_sync));

    /* Fill PDU */
    memcpy(pkt->pdu.adv_ind.adva, adv_address, sizeof(adv_address));
    memcpy(pkt->pdu.adv_ind.advd, advd, advd_size);
    
    pkt->pdu.adv_ind.header.bytes[0] = PDU_HDR_TYPE_ADV_NONCONN_IND;
    pkt->pdu.adv_ind.header.bytes[1] = sizeof(adv_address) + advd_size;

    /* Get the address for CRC */
    crc_ptr = pkt->pdu.adv_ind.adva + pkt->pdu.adv_ind.header.bytes[1];

    /* Packet length for later use */
    pkt->len = PREAMBLE_LEN + ACCESS_ADDRESS_LEN + PDU_HEADER_LEN +
            pkt->pdu.adv_ind.header.bytes[1] + CRC_LEN;

    /* Calculate CRC */
    ble_calc_adv_crc((u8 *)&(pkt->pdu),
            PDU_HEADER_LEN + pkt->pdu.adv_ind.header.bytes[1], crc_ptr);
    /* Whitening */
    ble_fast_whiten((u8 *)&(pkt->pdu),
            pkt->len - PREAMBLE_LEN - ACCESS_ADDRESS_LEN, 0, channel);
}

/**
 * Compose ADV_IND packet, calculate CRC and whiten the data.
 */
void ble_gen_adv_ind_pkts(u8 *advd, u8 advd_size)
{
    u8 channel;
    
    for (channel = 37; channel <= 39; channel++) {
        ble_gen_adv_ind_pkt(adv_ind_pkts[channel - 37], advd, advd_size, channel);
    }
}

/**
 * Send a BLE packet.
 *
 * @param pkt The BLE packet.
 */
void ble_send_pkt(BLE_PKT *pkt)
{
    ble_tx((u8 *)pkt, pkt->len);
}

/**
 * Send ADV_IND packet.
 */
void ble_send_adv_ind_pkt(u8 channel)
{
    ble_send_pkt(adv_ind_pkts[channel - 37]);
}

/**
 * Initialize BLE packets
 */
void ble_pkts_init(void)
{
}
