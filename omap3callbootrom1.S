# HKVCs simple omap3callbootrom1 for linboothkvc checking
# v15Jan2012_0130
# HKVC, GPL, 05Jan2012_2049
#
# This version successfully loads a new linux kernel and
# inturn a new linux system from with in a running linux
# system. It uses x-loader to bootstrap the new linux.
# Rather it is aganostic of whether one uses x-loader
# or something else. It is only that I by default use a
# x-load.bin.
#
# This also prints to uart.
# And disable MMU and sync data and instruction operations
# This also takes care of setting the uart base address
# based on currently selected device.
# The Maskbits in CPSR related to Interrupt, FIQ and Async
# abort is also handled.
# It has been found that the bootrom (atleast in qemu for now)
# expects the code to run be already loaded at 0x4020.0800
# This version copies the passed NirvanaChild code from the
# new version of lbhkvc_km.do to 0x4020.0800 and then
# This jumps to boot rom at 0x4001.4000
#

#
# r0 - Contains the Physical address where Nirvana's child is
# r1 - Contains the length of Nirvana's child in bytes
#

.text
.globl _start
_start:
	mov r0,r0
	mov r0,r0
	mov r0,r0
	mov r0,r0
	mov r0,r0
	mov r0,r0
	mov r0,r0
	mov r0,r0
	DSB
	mov r0,r0
	mov r0,r0
	cpsid iaf
	mov r0,r0
	ISB

	mrc p15, 0, r7, c1, c0, 0
	mrc p15, 0, r6, c1, c0, 0
	mov r0,r0
	mov r0,r0
	bic r7,r7,#1
	mcr p15, 0, r7, c1, c0, 0
	mov r0,r0
	mov r0,r0
	mov r0,r0
	ISB
	mov r0,r0
	mrc p15, 0, r8, c1, c0, 0


	mov r0,r0
	mov r0,r0
	lsrs r1, r1, #2
	add r1, r1, #4
	movw r2,#0x0800
	movt r2,#0x4020
moredata:
	ldr r3,[r0]
	str r3,[r2]
	add r0, r0, #4
	add r2, r2, #4
	subs r1, r1, #1
	bne moredata

	mov r4,#200
msgloop:
	mov r0,r0
	mov r0,r0

	ldr r1,uartBase
	mov r3,#100
# This loop waits till tx logic is ready by polling lsr
1:
	ldr r2,[r1, #0x14]
	subs r3,r3,#1
	beq 2f
	ands r2,#0x3
	bne 1b
2:
	mov r3,#0x31
	str r3,[r1]


	mov r0,r0
	mov r0,r0
	mov r0,r0
	mov r0,r0
	mov r0,r0
	mov r0,r0
	mov r0,r0
	mov r0,r0
	subs r4,r4,#1
	bne msgloop

#ifdef DEVICE_NOOKTAB
#error "NOT SUPPORTED CURRENTLY"
1:
	b	1b
#else
	movw r0,#0x4000
	movt r0,#0x4001
	mov pc,r0
#endif

uartBase:
#ifdef DEVICE_NOOKTAB
	.long 0x4806A000
#else
	.long 0x49020000
#endif

