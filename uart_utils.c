/*
 * uart_utils - Logic to directly write to uart0 of omap4 platform
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
	while(readw(uartPort+HKVC_UART_LSR) & TX_BUSY_MASK) {
#ifdef ENABLE_INSERTE_ONWAIT
		if(cnt == UARTWAIT_INSERTECNT) {
			writew('E',uartPort+HKVC_UART_TX);
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
}

