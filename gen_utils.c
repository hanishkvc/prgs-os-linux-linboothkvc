/*
 * gen_utils - Misc utilities required
 * v06Jan2012_2322
 * HKVC, GPL
 *
 */

#include "gen_utils.h"

void hkvc_sleep(unsigned long cnt)
{
	volatile unsigned long i=0;
	volatile unsigned long j;
	for(i=0; i<cnt; i++)
		j = i + j;
}

