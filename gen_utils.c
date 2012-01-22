/*
 * gen_utils - Misc utilities required
 * v20Jan2012_2102
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

unsigned long va2pa_cpr(unsigned long vAddr)
{
	unsigned long pAddr;

	__asm__ (
		"MCR p15, 0, %0, c7, c8, 0 \n"	/* CPR */
		"ISB \n"
		"MRC p15, 0, %1, c7, c4, 0 \n"
		: "=r"(pAddr)
		: "r"(vAddr)
		);
	return pAddr;
#warning "NOTE: va2pa_cpr doesn't mask the attributes in the 12 lsbits..."
}

unsigned long va2pa_cur(unsigned long vAddr)
{
	unsigned long pAddr;

	__asm__ (
		"MCR p15, 0, %0, c7, c8, 2 \n"	/* CUR */
		"ISB \n"
		"MRC p15, 0, %1, c7, c4, 0 \n"
		: "=r"(pAddr)
		: "r"(vAddr)
		);
	return pAddr;
#warning "NOTE: va2pa_cur doesn't mask the attributes in the 12 lsbits..."
}

