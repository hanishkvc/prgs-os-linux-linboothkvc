/*
 * uart_utils - Logic to directly write to uart of omap3/4 platform
 * v06Jan2012_2322
 * HKVC, GPL
 *
 * This assumes that the uart0 has already been mapped to iomem
 * cross check by looking into /proc/iomem of a running system.
 *
 */

#include <linux/kernel.h>
#include <linux/kernel.h>
#include <linux/io.h>
#include <linux/serial_reg.h>
#include <linux/compiler.h>
#include "uart_utils.h"
#include "gen_utils.h"

void __iomem *uartPort = (void *)0;

void hkvc_uart_init(void)
{
	uartPort = ioremap_nocache(DEBUG_UART_PBASE,32);
	BUG_ON(!uartPort);
	printk(KERN_INFO "lbhkvc: debug uart port base = 0x%p\n",uartPort);
}

void hkvc_uart_wait_on_tx_busy(void)
{
	int cnt = 0;
	hkvc_sleep(0x200);
	while(readw(uartPort+HKVC_UART_LSR) & TX_BUSY_MASK) {
#ifdef ENABLE_INSERTE_ONWAIT
		if(cnt == UARTWAIT_INSERTECNT) {
			writew('~',uartPort+HKVC_UART_TX);
		}
#endif
		if(cnt > UARTWAIT_MAXCNT) {
			break;
		}
		hkvc_sleep(0x10);
		cnt += 1;
	}
}

void hkvc_uart_send(char *buf, int len)
{
	unsigned long curIER;
	int i;

	/* Disable interrupts, so that we dont impact other uart based logics much */
	curIER = readw(uartPort+HKVC_UART_IER);
	writew(0,uartPort+HKVC_UART_IER);

	for(i=0; i<len; i++) {
		hkvc_uart_wait_on_tx_busy();
		writew(buf[i],uartPort+HKVC_UART_TX);
	}
	hkvc_uart_wait_on_tx_busy();
	writew(curIER,uartPort+HKVC_UART_IER);
	hkvc_sleep(0x400);
}

static char tBin2Hex[16] = { '0','1','2','3','4','5','6','7','8','9','A','B','C','D','E','F' };

void hkvc_uart_send_hex(unsigned long lData)
{
	unsigned long lMask = 0x0000000F;
	char sBuf[16];
	int i;

	sBuf[0] = '0';
	sBuf[1] = 'x';
	sBuf[10] = 0;
	for(i = 9; i > 1; i--) {
		sBuf[i]=tBin2Hex[lData&lMask];
		lData >>= 4;
	}
	hkvc_uart_send(sBuf,10);
}

