#!/bin/bash
adb shell cat /proc/kallsyms > target_kallsyms
adb push lbhkvc_k.ko /data/local/tmp/
cat lbhkvc_k.c | grep "kh_" | grep "= 0x"
cat target_kallsyms | grep "setup_mm_for_reboot"
cat target_kallsyms | grep "cpu_v7_proc_fin"
cat target_kallsyms | grep "cpu_v7_reset"
cat target_kallsyms | grep "v7_coherent_kern_range"
cat target_kallsyms | grep "disable_nonboot_cpus"
cat target_kallsyms | grep "show_pte"

