/*
 * uart_utils - Logic to directly write to uart of omap3/4 platform
 * v06Jan2012_2322
 * HKVC, GPL
 *
 * This assumes that the uart0 has already been mapped to iomem
 * cross check by looking into /proc/iomem of a running system.
 *
 */

#ifndef _HKVC_UART_UTILS_
#define _HKVC_UART_UTILS_


#define ENABLE_INSERTE_ONWAIT 1

#ifdef DEVICE_NOOKTAB
#define DEBUG_UART_PBASE	0x4806A000
//#define DEBUG_UART_VBASE	0x4806A000
#else
#ifdef DEVICE_BEAGLEXM
#define DEBUG_UART_PBASE	0x49020000
#else
#error "No VALID DEVICE defined"
#endif
#endif

#define TX_BUSY_MASK (UART_LSR_TEMT | UART_LSR_THRE)


#define HKVC_UART_LSR (UART_LSR<<2)
#define HKVC_UART_IER (UART_IER<<2)
#define HKVC_UART_TX (UART_TX<<2)

#define UARTWAIT_INSERTECNT 100
#define UARTWAIT_MAXCNT 200

extern void __iomem *uartPort;

void hkvc_uart_init(void);
void hkvc_uart_wait_on_tx_busy(void);
void hkvc_uart_send(char *buf, int len);
void hkvc_uart_send_hex(unsigned long lData);

#endif

