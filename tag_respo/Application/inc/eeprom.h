#ifndef _eeprom_h_
#define _eeprom_h_

#include "stm8l15x.h"

void get_id_information(void);
void write_eeprom_halfword(u32 address,u16 value);
void save_error_msg(u8 type);

#endif
