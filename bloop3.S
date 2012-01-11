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
#	.long 0x4806A000
	.long 0x49020000

