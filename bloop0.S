# bloop0.S - Recovered Initial Simple delay logic with jump to reset vector
# HKVC, GPL, Jan2012
# Stupid kernel make rules delete .s Asm files so had to recover from
# the binary code.
# Note this has the flag setting (rather NOT) mistake fixed in newer
# versions/programs.
# Also note that the simple jump to reset vector won't work in newer SOCs
# where the reset vector can be remapped from 0 to ffff.0000 if required.
#

00000000 <.data>:
   0:	e1a00000 	nop			; (mov r0, r0)
   4:	e1a00000 	nop			; (mov r0, r0)
   8:	e1a00000 	nop			; (mov r0, r0)
   c:	e1a00000 	nop			; (mov r0, r0)
  10:	e1a00000 	nop			; (mov r0, r0)
  14:	e1a00000 	nop			; (mov r0, r0)
  18:	e1a00000 	nop			; (mov r0, r0)
  1c:	e1a00000 	nop			; (mov r0, r0)
  20:	e1a00000 	nop			; (mov r0, r0)
  24:	e1a00000 	nop			; (mov r0, r0)
  28:	e1a00000 	nop			; (mov r0, r0)
  2c:	e1a00000 	nop			; (mov r0, r0)
  30:	e1a00000 	nop			; (mov r0, r0)
  34:	e1a00000 	nop			; (mov r0, r0)
  38:	e1a00000 	nop			; (mov r0, r0)
  3c:	e1a00000 	nop			; (mov r0, r0)
  40:	e1a00000 	nop			; (mov r0, r0)
  44:	e1a00000 	nop			; (mov r0, r0)
  48:	e1a00000 	nop			; (mov r0, r0)
  4c:	e1a00000 	nop			; (mov r0, r0)
  50:	e3a01ffa 	mov	r1, #1000	; 0x3e8
  54:	e3a02ffa 	mov	r2, #1000	; 0x3e8
  58:	e1a00000 	nop			; (mov r0, r0)
  5c:	e1a00000 	nop			; (mov r0, r0)
  60:	e1a00000 	nop			; (mov r0, r0)
  64:	e1a00000 	nop			; (mov r0, r0)
  68:	e2422001 	sub	r2, r2, #1
  6c:	1afffff9 	bne	0x58
  70:	e1a00000 	nop			; (mov r0, r0)
  74:	e2411001 	sub	r1, r1, #1
  78:	1afffff5 	bne	0x54
  7c:	e3a00000 	mov	r0, #0
  80:	e3a01001 	mov	r1, #1
  84:	e3a02002 	mov	r2, #2
  88:	e3a03003 	mov	r3, #3
  8c:	e3a04004 	mov	r4, #4
  90:	e3a05005 	mov	r5, #5
  94:	e3a00000 	mov	r0, #0
  98:	e1a0f000 	mov	pc, r0
