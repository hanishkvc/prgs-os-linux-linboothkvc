# HKVCs simple busy loop with no exit for linboothkvc checking
# v06Jan2012_1404
# HKVC, GPL, 05Jan2012_2049
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
	ISB

	mrc p15, 0, r7, c1, c0, 0
	mrc p15, 0, r4, c1, c0, 0
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
	mov r0,r0
	mov r0,r0

	ldr r1,uartBase
	mov r3,#100
# This loop waits till tx logic is ready by polling lsr
1:
	ldr r2,[r1, #0x14]
	sub r3,r3,#1
	beq 2f
	and r2,#0x3
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
	b _start

uartBase:
#ifdef DEVICE_NOOKTAB
	.long 0x4806A000
#else
	.long 0x49020000
#endif

