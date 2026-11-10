#!/bin/sh
# Resets Buffee through the J-Link's nSRST (run from this machine). The JTAG header has a Reset pin; needs openocd (brew install openocd / apt install openocd).
openocd -f interface/jlink.cfg -c "transport select jtag" -c "adapter speed 1000" -c "reset_config srst_only srst_push_pull" -c "adapter srst pulse_width 100" \
        -c "init" -c "adapter assert srst" -c "sleep 200" -c "adapter deassert srst" -c "sleep 50" -c "shutdown" 2>&1 | grep -E "VTarget|srst|Error|error" 
