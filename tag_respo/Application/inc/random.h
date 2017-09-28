#ifndef _random_h_
#define _random_h_

#include	"stm8l15x.h"
#include	"stdlib.h"
#include	"time.h"

void randtime_init(void);
u16 get_rand_time(void);

#endif