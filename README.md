<a rel="license" href="http://creativecommons.org/licenses/by-nc-sa/4.0/"><img alt="Creative Commons Licence" style="border-width:0" src="https://i.creativecommons.org/l/by-nc-sa/4.0/88x31.png" /></a>

This work is licensed under a <a rel="license" href="http://creativecommons.org/licenses/by-nc-sa/4.0/">Creative Commons Attribution-NonCommercial-ShareAlike 4.0 International License</a>.

# The Buffee Accelerator

PJIT is part of [the Buffee Project](https://www.buffee.ca). The hardware can be viewed [here](https://oshwlab.com/Renee/buffee-accelerator).

## Building

Download a copy of Code Composer Studio 10 (this might be buildable with GCC directly, but I'm not bothering with that right now). Git clone this into your workspace and then import it. You should be able to compile and debug from there.

## Building the Test

1. Clone the git
2. Cd into the cpu/ops folder
3. Run 'make'
4. Cd into the cpu folder
5. Run 'make'
6. Run 'm68k_test'
7. Watch your PC explode
