# tools/

| directory | for |
|---|---|
| [`gen/`](gen) | the table generator `make` runs, with its emitters; [`gen/check/`](gen/check) proves it right against a 68000 oracle |
| [`data/`](data) | generators of the data files that are checked in (opcode tables, test kernels, pace table), and the opcode profiles |
| [`pack/`](pack) | the bundle packer for the bootloader |
| [`buffee/`](buffee) | installing, running, soaking and probing a Buffee board; the GreenPAK and PRU build steps |
| [`bbb/`](bbb) | running and timing images on a BeagleBone Black |
| [`amiga/`](amiga) | the differential harness against an independent 68000 |
| [`study/`](study) | the models and measurements the design decisions came from |
| `wiki.py` | publishes `docs/` as a GitHub wiki |

Every tool, one line each: **[docs/tools.md](../docs/tools.md)**. Python tools run with the project's virtual environment (`make venv`) from the repository root.
