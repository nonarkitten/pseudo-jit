# The Buffee Accelerator

PJIT is part of [the Buffee Project](https://www.buffee.ca).

<a rel="license" href="http://creativecommons.org/licenses/by-nc-sa/4.0/"><img alt="Creative Commons Licence" style="border-width:0" src="https://i.creativecommons.org/l/by-nc-sa/4.0/88x31.png" /></a>

This work is licensed under a <a rel="license" href="http://creativecommons.org/licenses/by-nc-sa/4.0/">Creative Commons Attribution-NonCommercial-ShareAlike 4.0 International License</a>. No part of this shall be used for any commercial gains without express and written permission from The Buffee Project.

Please visit [the Wiki](https://github.com/nonarkitten/pseudo-jit/wiki) for more information.

## Future Plans

While PJIT is working in principle, getting it loaded and debugging has been a huge challenge with open source offerings (though, to be fair, they're actually better than TI's own Code Compser Studio). As such we're refactoring PJIT into PJIT 2.0 with the following goals:
- migrate old PJIT into pjit-legacy branch (to-do)
- migrating the Buffee Bootloader code to this repository (done)
- cleaning up instructions for Visual Studio Code for loading and debugging

## Single Binary PJIT
This meant making the whole executable under 127KB! This is incredibly small, but in our investigation, after many attempts at compressing code, we found that the generator itself was the smallest, by far. While making debugging much more accessible, it also makes booting much faster as our goal is to boot Buffee inside the 150ms window of the original 68000 (thus appearing virtually "instant").

So, during boot, PJIT has to:
- initialize all hardware including I2C, PMIC, clocks, EEPROM, DRAM, GPMC, GreenPAK, SPI and UART (done)
- verify basic operating state of the system (i.e. Power-On Self Test or POST; to-do)
- based on EEPROM settings, initialize PJIT cache and opcode jump tables (to-do)
- finally, start PJIT

Right now the bootloader is home to PJIT, mcl68k and a copy of tinyBASIC and we're still only at 100kB. We'll probably be removing mcl68k once PJIT is more stable which will clear up around 20kB for 680x0 opcodes.

## Building

We've moved to using Visual Studio Code -- a totally cross-platform development environment that's been working great on my Mac. Code Composer Studio was just too much to fight with. You'll need:

- Visual Studio Code (latest version)
- Cortex-debug extension (latest version)
- ARM GNU Embedded Toolchain (either 12.2 or 8.3.1)
- Segger J-Link (V6.32i ONLY)

You'll need to stick with J-Link V6.32i as it seems to be the last version which reliably debugs big-endian code. GCC only supported the BE8 flag from 8.3.1 and onward, but versions 9 thru 11 produce oddly larger than necessary code. Our original testing was with 8.3.1 but we're using 12.2 now and it seems fine (knock on wood).

I've included my .vscode/launch.json in the project which should "just work," as long as arm-none-eabi-* executables are in your path.

To build, open the shell and type "make." I'm not 100% this works on Windows anymore, but it should on Linux and Mac. This is now a one-step make that will build everything required and spit out the binary at the end.

## Programming -- UART
You'll need a good Terminal emulator package. On MacOS, I cannot recommend Serial highly enough -- this is a great bit of software. On Windows, Tera Term is absolutely the best. On Linux you're on your own, but I'm sure there's something you can apt-get. Along with this, you'll need an FTDI USB to Serial adapter along with a Tag Connect TC2030 (or compatible). People comfortable with hacking can solder bodge wires directly to their board and use any sort of USB to UART bridge (or even use another PC old enough to have serial on board). Here's the pinout:

```
          _
         (_)   Alignment pin (GND)
         
3.3VDC   o o   I2C SCL
FTDI Rx  o o   FTDI Tx
GND      o o   I2C SDA
        _   _
       (_) (_) Alignment pins (GND)
```

To program connect everything and power-on Buffee. If Buffee is uninitialized you should simply see the letter C repeating indefinately. If Buffee has already been programmed, you'll see the POST along with a small menu to perform more functions. To reprogram Buffee, simply select 'E' to erase the SPI flash, then reboot; from here you'll get the repeating C's.

To send the image, select XMODEM and the .BIN file generated when compiling PJIT (also provided here under releases). Once it uploads, you'll see the above menu again. To make the image permanent, select 'P' to program the SPI flash with the current version of PJIT.
```
[I2C0] Scanning bus..
[I2C0] 101_0000 ($50) EEPROM Detected
[I2C0] 000_10xx ($8~$A) GreenPAK Detected
[I2C0] GreenPAK Protection Bits: $00 $00 $00
[I2C0] 010_0100 ($24) PMIC Detected, Nitro mode enabled
[BCLK] Main bus clock measured at 7.158
[GPMC] Trimming Core PLL to: 966
[GPMC] SYNC VIOLATION: t->CSOFFTIME >= (t->ACCESSTIME+1)
[GPMC] Cycle Time: 27
[GPMC] Access Time: 25
[GPMC] nCS Timing (ON/OFF): 3/25
[GPMC] nRE Timing (ON/OFF): 0/26
[GPMC] nWE Timing (ON/OFF): 5/26
[BOOT] Build Date Apr  6 2023, Time 00:52:00
[BOOT] Image 402F0400 ~ 402FEABC (59068 bytes)
[BOOT] Completed in 12.45209 seconds

MENU
----
TESTS:
 1. Quick-test DDR memory
 2. Dump first 4K of SPI Flash
 3. Quick-test SPI flash
 4. Test GPMC
 5. Test printf
 6. Scan I2C Bus
 7. Run Native BogoMIPS test
 8. Run PJIT BogoMIPS test
 9. Scan and verify GreenPAK
SETUP:
 J. Jump to PJIT
 C. Set E Clock Divider
 G. Manage GreenPAK
 E. Erase SPI flash
 P. Program SPI flash
 H. Print help (this)
 X. Reboot
Ready
] █
```
Note: while rare, we may have to update the GreenPAK code. Presently, this is broken and you'll need to use an Arduino connected through the two pins on the TC-2030 header.

Note 2: yes, the I2C pins allow you direct access to the EEPROM, PMIC and GreenPAK.

## Programming -- JTAG
To program via JTAG you need to have some JTAG Programmer such as a Segger JLINK (or clone) along with the 20-pin adapter and Tag Connect TC-2050. As with the UART method, if you feel comfortable with it, you're welcome to hack together any other solution you like. Here's the pinout for the JTAG header on Buffee:
```
          _
         (_)   Alignment pin (GND)
         
3.3VDC   o o   Reset
TMS      o o   TRST
GND      o o   TDI
TCK      o o   GND
GND      o o   TDO
        _   _
       (_) (_) Alignment pins (GND)
```
From Visual Studio Code, simply hit the "Run" icon to then start debugging.

Note on both Tag Connect headers: there are no "click" holes to hold the Tag Connect pogo pins firmly in place for prolonged programming times. My recommendation is to use ouchless hair elastics. Wrap the elastic around Buffee length-wise before replacing it into the 68000 socket and then loop that around the Tag Connect header at the taper (strain releif) part. You may have to play around with the tension to make it stand up right, but once you do, it should stay firm indefinately. I know this sounds supremely hacky, but it's actually better than trying to use their own "pin anchors".

If you Flash to SPI and something bad happens, you can ONLY recover via JTAG on release v0.1. The next release will have an emergency erase that's triggered by holding down the ESC key in your terminal during boot. This will then allow you to re-upload a new image to RAM.
