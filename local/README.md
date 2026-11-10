# local/

Everything in this directory except this file is git-ignored: it is yours.

| directory | what goes here |
|---|---|
| `roms/` | Kickstart ROM images for the harnesses (`tools/amiga/`, `tools/study/rom_scan.py`). **Never commit a ROM.** |
| `traces/` | instruction traces the studies in `tools/study/` read (`trace_dhrystone_*.bin`, `trace_tinybasic.bin`, the LhA traces of `tools/study/trace_lha.sh`); regenerate them with the tests that write them. |
| `firmware/` | known-good bootloader and Kickstart-bundle images you flashed to a board and want to be able to flash again, for example a copy of `bootloader.bin` and `buffee_boot.lzt`. `tools/buffee/restore_*.py --image/--bundle` take any path. |
| `history-candidates/` | the two ways of publishing the project without the earlier history's third-party material, as bare repositories, and the script that makes them (its README says how to use them). |
