#!/usr/bin/env python3
"""One triggered capture with Saleae Logic 2's automation server (run on the machine the Saleae is plugged into;
Logic 2 running with 'Enable automation server' on).
  pip3 install logic2-automation
  logic_capture.py out_dir [--rate 24000000] [--channels 0,1,2,3,4,5,6,7] [--trig 1] [--edge falling] [--after
  0.004] [--timeout 20]
Waits for the trigger (e.g. ASn falling), records `after` seconds past it, exports the digital data to
out_dir/digital.csv (time, one column per channel, a row per change).
"""

import argparse, os
from saleae import automation

ap = argparse.ArgumentParser()
ap.add_argument("out")
ap.add_argument("--rate", type=int, default=24_000_000)
ap.add_argument("--channels", default="0,1,2,3,4,5,6,7")
ap.add_argument("--trig", type=int, default=1)
ap.add_argument("--edge", default="falling", choices=["rising", "falling"])
ap.add_argument("--after", type=float, default=0.004)
ap.add_argument("--timeout", type=float, default=30.0)
ap.add_argument(
    "--trim", type=float, default=0.0002, help="trigger mode: keep only this much data before the trigger"
)
ap.add_argument("--manual", type=float, default=0.0, help="no trigger: record this many seconds from now")
ap.add_argument("--port", type=int, default=10430)
a = ap.parse_args()
import signal

signal.alarm(int(a.timeout) + 5)
chans = [int(x) for x in a.channels.split(",")]
os.makedirs(a.out, exist_ok=True)
with automation.Manager.connect(port=a.port) as mgr:
    dev = automation.LogicDeviceConfiguration(enabled_digital_channels=chans, digital_sample_rate=a.rate)
    if a.manual:
        mode = automation.ManualCaptureMode()
    else:
        mode = automation.DigitalTriggerCaptureMode(
            trigger_type=(
                automation.DigitalTriggerType.FALLING
                if a.edge == "falling"
                else automation.DigitalTriggerType.RISING
            ),
            trigger_channel_index=a.trig,
            after_trigger_seconds=a.after,
            trim_data_seconds=a.trim,
        )
    cfg = automation.CaptureConfiguration(capture_mode=mode)
    with mgr.start_capture(device_configuration=dev, capture_configuration=cfg) as cap:
        print("armed", flush=True)
        if a.manual:
            import time

            time.sleep(a.manual)
            cap.stop()
        cap.wait()
        cap.export_raw_data_csv(directory=a.out, digital_channels=chans)
print("captured to", a.out)
