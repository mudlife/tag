#ifndef	_watchdog_h_
#define _watchdog_h_

#include "stm8l15x.h"

void wwdg_init(void);
void wwdg_feed(void);

#endif
