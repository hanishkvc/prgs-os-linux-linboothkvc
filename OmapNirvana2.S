# HKVCs simple OmapNirvana2 for linboothkvc checking
# v21Jan2012_2254
# HKVC, GPL, 05Jan2012_2049
#
# This version successfully loads a new linux kernel and
# inturn a new linux system from with in a running linux
# system. It uses x-loader to bootstrap the new linux.
# Rather it is aganostic of whether one uses x-loader
# or something else. It is only that I by default use a
# x-load.bin.
#
# This also prints to uart to help debug.
# The Maskbits in CPSR related to Interrupt, FIQ and Async
# abort is also handled.
# It disables MMU and syncs data and instruction operations
# This also takes care of setting the uart base address
# based on currently selected device.
# The version copies the passed NirvanaChild code to the
# required destination location based on device selected
# and passes control to it after appropriately handling
# required stuff.
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

	ldr r9,uartVBase
	ldr r10,uartBase
	mov r11,#0x35
	str r11,[r9]
	str r11,[r10]

	# Sync all memory operations till now
	DSB
	mov r0,r0
	mov r0,r0

	mov r11,#0x34
	str r11,[r9]
	str r11,[r10]

	# Disable/Mask interrupts, fiqs, and Async Aborts
	cpsid iaf
	mov r0,r0
	ISB

	mov r11,#0x33
	str r11,[r9]
	str r11,[r10]

	# Disable MMU
	mov r0,r0
	mov r0,r0
	mov r0,r0
	mov r0,r0
	mrc p15, 0, r7, c1, c0, 0
	mrc p15, 0, r6, c1, c0, 0
	mov r0,r0
	mov r0,r0
	bic r7,r7,#1
	mcr p15, 0, r7, c1, c0, 0
	ISB
	mov r0,r0
	mov r0,r0
	mov r0,r0
	mov r0,r0

	#NOTE: The MMU is disabled so the Uart ioremapped address is no longer valid
	# writing to the virtual address will raise a exception, which we aren't currently
	# handling, so it will NUKE the Nirvana env.
	# THATS why I no longer write to [r9] below
	mov r11,#0x32
	str r11,[r10]
	#str r11,[r9]
	str r11,[r10]
	#str r11,[r9]

	mrc p15, 0, r8, c1, c0, 0

	mov r0,r0
	mov r0,r0
	mov r0,r0
	mov r0,r0

#ifdef DEVICE_NOOKTAB
	# Do any device specific things like shutdown any watchdog etc...

#else
	# This is for BeagleXM

#if 0
	# Stop WatchDog Timer 2 (MPU Watchdog) 48314000
	movw r2,#0x4000
	movt r2,#0x4831
	# WSPR - Start Stop register
	movw r3,#0xAAAA
	movt r3,#0x0000
	str r3,[r2, #0x48]

	# WWPS - Write Postings
1:	ldr r4, [r2, #0x34]
	cmp r4,#0
	bne 1b

	movw r3,#0x5555
	movt r3,#0x0000
	str r3,[r2, #0x48]

	# WWPS - Write Postings
1:	ldr r4, [r2, #0x34]
	cmp r4,#0
	bne 1b


	# WLDR - Load register
	movw r3,#0x0001
	movt r3,#0x1111
	str r3,[r2, #0x2C]

	#WTGR - Trigger register
	movw r3,#0x1a11
	movt r3,#0x111a
	str r3,[r2, #0x30]
#endif
#endif

	mov r0,r0
	mov r0,r0
	mov r0,r0
	mov r0,r0

	# Move NChild to final location
	lsrs r1, r1, #2
	add r1, r1, #4
#if defined(DEVICE_PANDA)
	# This is for Omap4 based devices with my modified Ti X-loader as NChild
	movw r2,#0x4350
	movt r2,#0x4030
#elif defined(DEVICE_NOOKTAB)
	# Note that in NookTAB may be because of HS nature,
	# may be 40304350 and around is used by HS code
	# writing to 0x4030.4350 triggers a exception potentially,
	# have to debug later
	movw r2,#0x7ff0
	movt r2,#0x8000
#else
	# This is for Omap3 (Omap3[4|6]30, Dm3730,....)
	movw r2,#0x0800
	movt r2,#0x4020
#endif
	mov r11,#0x31
	str r11,[r10]
moredata:
	ldr r3,[r0]
	#mov r11,#0x34
	#str r11,[r10]
	str r3,[r2]
	#mov r11,#0x33
	#str r11,[r10]
	add r0, r0, #4
	add r2, r2, #4
	subs r1, r1, #1
	bne moredata

	# Print 0 to 9 on uart3
	mov r4,#10
	ldr r1,uartBase
	mov r7,#0x30
msgloop:
	mov r0,r0
	mov r0,r0
	mov r0,r0
	mov r0,r0

	mov r3,#100
	# This loop waits till tx logic is ready by polling lsr, or till 100 times
1:
	ldr r2,[r1, #0x14]
	subs r3,r3,#1
	beq 2f
	ands r2,#0x3
	bne 1b
2:
	str r7,[r1]
	add r7, r7, #1

	mov r0,r0
	mov r0,r0
	mov r0,r0
	mov r0,r0
	subs r4,r4,#1
	bne msgloop

	# Jump to NChild suitably
	mov r0,r0
	mov r0,r0
	mov r0,r0
	mov r0,r0

#if defined(DEVICE_NOOKTAB)
	# Base address of memory containing metadata
	movw r0,#0x7ff0
	movt r0,#0x8000
#elif defined(DEVICE_PANDA)
	# Base address of memory containing metadata
	movw r0,#0x4350
	movt r0,#0x4030
#endif

#if (defined(DEVICE_NOOKTAB) || defined(DEVICE_PANDA))
	# Boot device ==> 0x05 = MMC/SD1
	mov  r1,#0x05
	str  r1,[r0,#0x08]
	# Boot Mode ==> 0x02 = Fat Boot, 0x01 = Raw Boot
	mov  r1,#0x02
	str  r1,[r0,#0x0C]
	# The actual _start starts after this 16Byte header
	add  r1, r0, #0x10
	mov pc, r1
	# If it ever returns from NChild
1:
	b	1b
#else
	# Directly jump to NChild
	# Rather r2 should contain this address
	movw r0,#0x0800
	movt r0,#0x4020
	mov pc,r0
#endif

	# NOTE: FIXME: This hardcoding of Uart ioremapped address is WAPPER THIN BE CAREFULL
	# NOTE: Has to be passed as a argument from Kernel Module actually
uartVBase:
#ifdef DEVICE_NOOKTAB
	.long 0xfa06a000
#elif defined(DEVICE_BEAGLEXM)
	.long 0xfb020000
#elif defined(DEVICE_PANDA)
	.long 0xfa020000
#else
#error "UNKNOWN DEVICE..."
#endif

uartBase:
#ifdef DEVICE_NOOKTAB
	.long 0x4806A000
#elif defined(DEVICE_BEAGLEXM)
	.long 0x49020000
#elif defined(DEVICE_PANDA)
	.long 0x48020000
#else
#error "UNKNOWN DEVICE..."
#endif

