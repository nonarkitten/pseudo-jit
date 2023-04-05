# buffee_bootloader

The bootloader for the AM3358

## Purpose
The AM3358 has it's own ROM based bootloader. This will look on the SPI flash to load this, the second-stage bootloader. For our purposes, this is the only bootloader we need to be concerned with.

This will initialize all the hardware, perform basic testing and diagnosis and load the main PJIT program to DDR or SPI flash. It's also able to flash itself into SPI flash to boot from, but care must be taken as if this if buggy AT ALL, then you will brick your Buffee. Yay.

## Current Status

Most things are working; though not 100%.
```
              ____ ______________
  ______     |    |   \__    ___/
  \  __ \    |    |   | |    |
  |  |_) )\__|    |   | |    |
  |   __/\________|___| |____|
  |__|

[BOOT] Build Date Oct 21 2022, Time 17:49:02
[BOOT] Image 402F0400 ~ 402F600C (23564 bytes)
[I2C0] Scanning bus...
[I2C0] 000_10xx ($8~$A) GreenPAK Detected.
[I2C0] 010_0100 ($24) PMIC Detected, Nitro mode enabled.
[I2C0] 101_0000 ($50) EEPROM Detected.
[BOOT] Completed in 0.07710 seconds.
MENU
1. Quick-test DDR memory.
2. Dump first 4K of SPI Flash.
3. Quick-test SPI flash (Warning: destructive).
4. Erase whole SPI flash.
5. Program SPI flash with flash loader.
6. XMODEM-1K upload image to DDR.
7. Program SPI flash with image.
8. Test GPMC.
9. Test printf.
A. Run Native BogoMIPS test.
X. Reboot.
] 1
[DDR0] Read 01010101 from 80000000, expected  01010101
[DDR0] Read 02020202 from 80000004, expected  02020202
[DDR0] Read 04040404 from 80000008, expected  04040404
[DDR0] Read 08080808 from 8000000C, expected  08080808
[DDR0] Read 10101010 from 80000010, expected  10101010
[DDR0] Read 20202020 from 80000014, expected  20202020
[DDR0] Read 40404040 from 80000018, expected  40404040
[DDR0] Read 80808080 from 8000001C, expected  80808080
MENU
1. Quick-test DDR memory.
2. Dump first 4K of SPI Flash.
3. Quick-test SPI flash (Warning: destructive).
4. Erase whole SPI flash.
5. Program SPI flash with flash loader.
6. XMODEM-1K upload image to DDR.
7. Program SPI flash with image.
8. Test GPMC.
9. Test printf.
A. Run Native BogoMIPS test.
X. Reboot.
] 3
Are you sure [y/N]? y
[FLASH] Flash Device ID: 1f16
[FLASH] Erasing page 0
[FLASH] Writing pattern block
[FLASH] Read: FFFFFFFF AA55AA55 AA55AA55 AA55AA55
[FLASH] Writing zero block
[FLASH] Read: 00000000 00000000 00000000 00000000
[FLASH] Tests failed
MENU
1. Quick-test DDR memory.
2. Dump first 4K of SPI Flash.
3. Quick-test SPI flash (Warning: destructive).
4. Erase whole SPI flash.
5. Program SPI flash with flash loader.
6. XMODEM-1K upload image to DDR.
7. Program SPI flash with image.
8. Test GPMC.
9. Test printf.
A. Run Native BogoMIPS test.
X. Reboot.
] 8
[GPMC] ROM Check: exec 34.2 (28 Oct 1987)
[GPMC] Bytes NOT swapped
[GPMC] Raw bytes:
[GMPC] $00000000: 11 11 4e f9 00 fc 00 d2
[GMPC] $00000008: 00 00 ff ff 00 22 00 05
[GMPC] $00000010: 00 22 00 02 ff ff ff ff
[GMPC] $00000018: 65 78 65 63 20 33 34 2e
[GPMC] Read words (LE:BE)
[GMPC] $00000000: 1111:1111 f94e:4ef9 fc00:00fc d200:00d2
[GMPC] $00000008: 0000:0000 ffff:ffff 2200:0022 0500:0005
[GMPC] $00000010: 2200:0022 0200:0002 ffff:ffff ffff:ffff
[GMPC] $00000018: 7865:6578 6365:6563 3320:2033 2e34:342e
[GPMC] Read longs (LE:BE)
[GMPC] $00000000: f94e1111:11114ef9 d200fc00:00fc00d2
[GMPC] $00000008: ffff0000:0000ffff 05002200:00220005
[GMPC] $00000010: 02002200:00220002 ffffffff:ffffffff
[GMPC] $00000018: 63657865:65786563 2e343320:2033342e
[GPMC] Performing benchmark with ROM (~1.5s)
[GPMC] Read 1048576 words in 1.35 s (1.47 MB/s)
[GPMC] JOYxDAT Check: 0xf0f0 0xf2f3 (should be close to 0xF0F0)
[GPMC] JOYxDAT Check: 0xf2f3 0xffff (should be close to 0xA5A5)
[GPMC] CIAA timer A Start: 0x00ffffff
[GPMC] CIAA timer A End: 0x00ffffff
[GPMC] CIAA timer A Elapsed: 0
MENU
1. Quick-test DDR memory.
2. Dump first 4K of SPI Flash.
3. Quick-test SPI flash (Warning: destructive).
4. Erase whole SPI flash.
5. Program SPI flash with flash loader.
6. XMODEM-1K upload image to DDR.
7. Program SPI flash with image.
8. Test GPMC.
9. Test printf.
A. Run Native BogoMIPS test.
X. Reboot.
] A
[TEST] Performing Native BogoMIPS benchmark, warming up
[TEST] Warm up complete, starting pass 1
[TEST] Native BogoMIPS 999.99
[TEST] Loops: 536870912, duration 1073 ms
MENU
1. Quick-test DDR memory.
2. Dump first 4K of SPI Flash.
3. Quick-test SPI flash (Warning: destructive).
4. Erase whole SPI flash.
5. Program SPI flash with flash loader.
6. XMODEM-1K upload image to DDR.
7. Program SPI flash with image.
8. Test GPMC.
9. Test printf.
A. Run Native BogoMIPS test.
X. Reboot.
] 
```
