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

void __iomem *uartport = (void *)0;

void hkvc_uart_init(void)
{
	uartport = ioremap_nocache(DEBUG_UART_PBASE,32);
	BUG_ON(!uartport);
	printk(KERN_INFO "lbhkvc: debug uart port base = 0x%p\n",uartport);
}

void hkvc_uart_wait_on_tx_busy(void)
{
	int cnt = 0;
	while(readw(uartport+HKVC_UART_LSR) & TX_BUSY_MASK) {
		if(cnt > 100)
			break;
		hkvc_sleep(0x10);
		cnt += 1;
	}

}

void hkvc_uart_send(char *buf, int len)
{
	unsigned long curIER;
	int i;

	/* Disable interrupts, so that we dont impact other uart based logics much */
	curIER = readw(uartport+HKVC_UART_IER);
	writew(0,uartport+HKVC_UART_IER); 

	for(i=0; i<len; i++) {
		hkvc_uart_wait_on_tx_busy();
		writew(buf[i],uartport+HKVC_UART_TX);
	}
	hkvc_uart_wait_on_tx_busy();
	writew(curIER,uartport+HKVC_UART_IER);
}

