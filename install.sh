#!/bin/bash
# HKVC, GPL, Jan2012

# This is avoided by using serial port and uuencode and xmodem
# thus avoiding additional cable, when uart is already used for debugging
# Due to non compliance of programs in ubuntu/unity to stick to defined
# settings had to avoid the adb route.
#

adb push lbhkvc_km_$DEVICE.ko /data/local/tmp/
read -p "Copied to target /data/local/tmp ...?"
