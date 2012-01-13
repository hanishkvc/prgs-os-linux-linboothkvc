# HKVCs simple busy loop with exit for linboothkvc checking
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
	mov r1,#8000
1:
	mov r2,#8000
2:
	mov r0,r0
	mov r0,r0
	mov r0,r0
	mov r0,r0
	subs r2,r2,#1
	bne 2b

	mov r0,r0
	mov r0,r0
	mov r0,r0
	mov r0,r0
	subs r1,r1,#1
	bne 1b
	
	mov r0,r0
	mov r0,r0
	mov r0,r0
	mov r0,r0
	mov r0,#8000
	mov pc,r0

