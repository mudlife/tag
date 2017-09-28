#include	"random.h"

void randtime_init(void)
{
	srand((unsigned)time(NULL));
}

u16 get_rand_time(void)
{
  u16 temp0,temp1;
  temp0=rand()%1000;
  temp1=rand()%17;
  return (temp0*temp1);
}

