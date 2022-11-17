# The Buffee Accelerator

PJIT is part of [the Buffee Project](https://www.buffee.ca).

<a rel="license" href="http://creativecommons.org/licenses/by-nc-sa/4.0/"><img alt="Creative Commons Licence" style="border-width:0" src="https://i.creativecommons.org/l/by-nc-sa/4.0/88x31.png" /></a>

This work is licensed under a <a rel="license" href="http://creativecommons.org/licenses/by-nc-sa/4.0/">Creative Commons Attribution-NonCommercial-ShareAlike 4.0 International License</a>. No part of this shall be used for any commercial gains without express and written permission from The Buffee Project.

Please visit [the Wiki](https://github.com/nonarkitten/pseudo-jit/wiki) for more information.

## Building

We've moved to using Visual Studio Code -- a totally cross-platform development environment that's been working great on my Mac. Code Composer Studio was just too much to fight with. To use, simple grab a copy of Visual Studio Code and install the cortex-debug extension and JLINK software from SEGGER. I've included my .vscode/launch.json in the project which should "just work."

To build, open the shell and type "make." I'm not 100% this works on Windows anymore, but it should on Linux and Mac. This is now a one-step make that will build everything required and spit out the binary at the end.
