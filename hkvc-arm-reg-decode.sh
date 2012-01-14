#!/usr/bin/python
#
# simple logic to decode the meaning of few arm registers
# HKVC, GPL, Jan2012
#

import sys

rName = sys.argv[1]
rVal = int(sys.argv[2],0)

def get_bits(rVal, bitMsb, bitLen):
	tMsb = bitMsb
	tLsb = bitMsb-bitLen+1
	if tMsb < 0:
		print "ERROR: tMsb {0}".format(tMsb)
		return -1
	if tLsb < 0:
		print "ERROR: tLsb {0}".format(tLsb)
		return -1

	tMask = 0
	for i in range(tLsb,tMsb+1):
		tMask = tMask | (1 << i)
	return ((rVal & tMask) >> tLsb)


def print_bits(sDesc, rVal, bitMsb, bitLen):
	print "{0} {1}".format(sDesc,hex(get_bits(rVal,bitMsb,bitLen)))


if rName == "cp15.sctlr":
	print "Info for {0} = 0x{1} = bin({2})".format(rName,hex(rVal),bin(rVal))
	print_bits( "31:31 - SBZP = ", rVal,31,1)
	print_bits( "30:30 - TE(ThumbExceptionEnable) = ", rVal,30,1)
	print_bits( "29:29 - AFE(AccessFlagEnable) = ", rVal,29,1)
	print_bits( "28:28 - TRE(TexRemapEnable) = ", rVal,28,1)
	print_bits( "27:27 - NMFI(NonMaskableFIQ support) = ", rVal,27,1)
	print_bits( "26:26 - SBZP = ", rVal,26,1)
	print_bits( "25:25 - EE(ExceptionEndianness) = ", rVal,25,1)
	print "NOTE: EE = 0 = LittleEndian AND EE = 1 = BigEndian"
	print_bits( "24:24 - VE(InterruptVectorsEnable) = ", rVal,24,1)
	print_bits( "23:23 - SBOP = ", rVal,23,1)
	print_bits( "22:22 - U(ARMv7=SBOP) = ", rVal,22,1)
	print_bits( "21:21 - FI(FastInterruptsConfigurationEnable) = ", rVal,21,1)
	print_bits( "20:20 - UWXN(WithVirt:UnprivilegedWritePermissionImpliesPL1XN) = ", rVal,20,1)
	print_bits( "19:19 - WXN(WithVirt:WritePermissionImpliesXN) = ", rVal,19,1)
	print "NOTE: 20:19 = SBZP for implementation with NO VirtExtensions"
	print_bits( "18:18 - SBOP = ", rVal,18,1)
	print_bits( "17:17 - HA (HardwareAccessFlagEnable) = ", rVal,17,1)
	print_bits( "16:16 - SBOP = ", rVal,16,1)
	print_bits( "15:15 - SBZP = ", rVal,15,1)
	print_bits( "14:14 - RR (RoundRobinSelect) = ", rVal,14,1)
	print_bits( "13:13 - V (VectorsBit) = ", rVal,13,1)
	print "NOTE: V = 0 = LowExceptionVectors(0x0) AND V = 1 = HiVecs = 0xFFFF0000"
	print_bits( "12:12 - I (Instruction CACHE Enable) = ", rVal, 12, 1)
	print_bits( "11:11 - Z (BranchPredictionEnable) = ", rVal, 11, 1)
	print_bits( "10:10 - SW (SWPandSWPBEnable) = ", rVal, 10, 1)
	print_bits( "09:08 - SBZP = ", rVal, 9, 2)
	print_bits( "07:07 - B (ARMv7=SBZP) = ", rVal, 7, 1)
	print_bits( "06:06 - SBOP = ", rVal, 6, 1)
	print_bits( "05:05 - CP15BEN (CP15BarrierEnable) = ", rVal, 5, 1)
	print_bits( "04:03 - SBOP = ", rVal, 4, 2)
	print_bits( "02:02 - C (CacheEnable(DataAndUnified)) = ", rVal, 2, 1)
	print_bits( "01:01 - A (AlignmentCheckEnable) = ", rVal, 1, 1)
	print_bits( "00:00 - M (MMUEnable) = ", rVal, 0, 1)


if rName == "cpsr":
	print "Info for {0} = 0x{1} = bin({2})".format(rName,hex(rVal),bin(rVal))


	print_bits( "31:31 - N(NegativeConditionFlag) = ", rVal,31,1)
	print_bits( "30:30 - Z(ZeroConditionFlag) = ", rVal,30,1)
	print_bits( "29:29 - C(CarryConditionFlag) = ", rVal,29,1)
	print_bits( "28:28 - V(OverflowConditionFlag) = ", rVal,28,1)
	print_bits( "27:27 - Q(CummulativeSaturationBit) = ", rVal,27,1)

	print_bits( "26:25 - ThumbIT(1:0) = ", rVal,26,2)
	print_bits( "24:24 - J(JazelleBit) = ", rVal,24,1)
	print_bits( "23:20 - SBZP = ", rVal,23,4)
	print_bits( "19:16 - GE(3:0)(GreaterThanOrEqual for SIMD) = 0x", rVal,19,4)
	print_bits( "15:10 - ThumbIT(7:2) = ", rVal,15,6)

	print_bits( "09:09 - E(EndiannessExecutionStateBit) = ", rVal,9,1)
	print "NOTE: E = 0 = LittleEndian AND E = 1 = BitEndian"
	print_bits( "08:08 - A(AsyncAbortMaskBit) = ", rVal, 8, 1)
	print_bits( "07:07 - I(IrqMaskBit) = ", rVal, 7, 1)
	print_bits( "06:06 - F(FiqMaskBit) = ", rVal, 6, 1)
	print_bits( "05:05 - T(ThumbExecutionStateBit) = ", rVal, 5, 1)
	print_bits( "04:00 - M(4:0)(ModeBit) = 0x", rVal, 4, 5)
	mode=get_bits(rVal,4,5);
	print "NOTE: M(4:0) means ..."
	if(mode == 0x10):
		print "MODE = User (PL0) "
	if(mode == 0x11):
		print "MODE = FIQ (PL1)"
	if(mode == 0x12):
		print "MODE = IRQ (PL1)"
	if(mode == 0x13):
		print "MODE = Supervisor(svc) (PL1)"
	if(mode == 0x16):
		print "MODE = Monitor (PL1) - Implemented with Security Extensions, State: Secure Only"
	if(mode == 0x17):
		print "MODE = Abort (PL1)"
	if(mode == 0x1a):
		print "MODE = Hyp (PL2) - Implemented with Virtualization Extensions, State: Non-secure Only"
	if(mode == 0x1b):
		print "MODE = Undefined[und] (PL1)"
	if(mode == 0x1f):
		print "MODE = System[sys] (PL1)"


