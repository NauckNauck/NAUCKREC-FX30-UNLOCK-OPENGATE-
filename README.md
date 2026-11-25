# NAUCKREC — Sony ILME-FX30 Open Gate (3:2) & V60 Unlock

100% volatile RAM-only research & tooling to enable **Open Gate 3:2 (`3840x2560`)**, **3:2 LCD preview**, and **V60 SD card unlock** on the **Sony ILME-FX30 (Firmware 6.01, BIONZ XR / IMX684)**.

> **Disclaimer:** Educational and interoperability research only. No proprietary Sony firmware binaries or dumps are included in this repository. All patches are 100% volatile in RAM (a 10-second battery pull restores stock behavior).

## Credits & Attribution

- **Author:** [NauckNauck](https://github.com/NauckNauck)
- **License:** [GNU GPLv3](LICENSE) — any fork, redistribution, or derivative work **must** credit the author and remain open-source under GPLv3.

## Highlights

- **Sensor / SGC (`fx30_ogsq_v6.c`):** Full 3:2 IMX684 readout (`5952x3968`), `CROPV = 96`, `Lrc_hscale` (`0x786`) preserved at stock `5623`, `Lrc_vphase` (`0x78c`) = `3726`, `PDAF_OFFS` (`+14`), `PDAF_END_OFFS` (`+634`), `TG_Node10` (`0x30`), and `0x49c/0x4ac/0x4bc` (`+310`).
- **3:2 LCD Preview (`fx30_outsz_lcd_v1.c`):** VDF panel fit mode (`0x001e2d38`, `0x001e2d50`: `3 -> 0`), `outsz` (`3840x2160 -> 3840x2560`), and AF/tracking overlay bounds.
- **DMMWB & Encoder (`fx30_dmmwb_safe_v2.c`, `build_fx30og_v3.py`):** Safe 71 MiB `0x6fa` allocation steering + `av-cam.bin` / `libmpr.so` patches.
- **Runner:** `fx30_open_gate/run_fx30_complete.py` (`--verify-offline`, `--step1`, `--full --with-dmmwb`, `--probe`).
