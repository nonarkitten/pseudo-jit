<a rel="license" href="http://creativecommons.org/licenses/by-nc-sa/4.0/"><img alt="Creative Commons Licence" style="border-width:0" src="https://i.creativecommons.org/l/by-nc-sa/4.0/88x31.png" /></a>

This work is licensed under a <a rel="license" href="http://creativecommons.org/licenses/by-nc-sa/4.0/">Creative Commons Attribution-NonCommercial-ShareAlike 4.0 International License</a>.

# The Buffee Accelerator

PJIT is part of [the Buffee Project](https://www.buffee.ca). The hardware can be viewed [here](https://oshwlab.com/Renee/buffee-accelerator).

## Building

Download a copy of Code Composer Studio 10 (this might be buildable with GCC directly, but I'm not bothering with that right now). Git clone this into your workspace and then import it. You should be able to compile and debug from there.

### Building PJIT Opcodes

This is how I did it -- there are many different paths to Nirvana here.
1. Enable Windows Subsystem for Linux (Turn Windows features on or off)
2. Download Ubuntu 18.04 LTS https://aka.ms/wsl-ubuntu-1804 and install it
3. If you want to make your life super easy, install:
    - VS Code (https://code.visualstudio.com/)
    - WSL Remote extension (https://marketplace.visualstudio.com/items?itemName=ms-vscode-remote.remote-wsl) 
4. Run Ubuntu and install the build-essentials: ```sudo apt install build-essential```
5. Go into the generator folder, e.g., ```cd /mnt/d/Workspaces/CCS10/pjit-private/cpu/ops```
6. And build, ```make -j16```

## Building PJIT

Once the opcodes are generated, then you'll be able to build PJIT and run it on Buffee.

## Building the Flash Loader

TBD
