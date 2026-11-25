# NAUCKREC — Sony ILME-FX30 Open Gate (3:2) & V60 Unlock

100% volatile RAM-only research & tooling to enable **Open Gate 3:2 (`3840x2560`)**, **LCD 3:2 preview**, and **V60 SD card unlock** on the **Sony ILME-FX30 (Firmware 6.01, BIONZ XR / IMX684)**.

> **Disclaimer:** Educational and interoperability research only. No proprietary Sony firmware binaries or dumps are included in this repository. All patches are 100% volatile in RAM (a 10-second battery pull restores stock behavior).

## Credits & Attribution

- **Author:** [NauckNauck](https://github.com/NauckNauck)
- **License:** [GNU GPLv3](LICENSE) — any fork, redistribution, or derivative work **must** credit the author and remain open-source under GPLv3.

## Current Verified Status

- **V60 Card Unlock (`fx30menu_v1` + `fx30rate_v1`):** Verified on hardware — unlocks **All-Intra 4K (XAVC S-I)** and **S&Q up to 240 fps** on V60 SD cards.
- **Open Gate 3:2 (`3840x2560` 4:2:2 10-bit AVC / XAVC S 4K):** Verified on hardware (`NAUCK.REC0043` / `NAUCK.REC0058` / `TEST_0057_3840x2560.MP4`) — decoded H.264 SPS confirms real `3840x2560` (`crop=(0,0,0,0)`), accurate colors, zero vertical banding.
- **Lossless MP4 Container Fixer (`scripts/fix_mp4_2560.py`):** Instant zero-reencode `avc1`/`tkhd` container box patcher (`3840x2160 -> 3840x2560`) for NLEs/players.
- **In Progress / Known Limitations (see [`docs/FX30_AUDITORIA_E_STATUS_ATUAL.md`](docs/FX30_AUDITORIA_E_STATUS_ATUAL.md)):**
  - `build_fx30og_v3.py` (`1b63fdff...`) added `0x180098` & `0x109768` in `libmpr.so` (hardware check pending; `libXavcMp4` also hardcodes `#0x870` at `0x21ff68`/`0x2200a0`/`0x21fb34`).
  - `fx30_outsz_lcd_v1.c` fills FX30's 4:3 physical LCD panel (`+12.5%` vertical stretch; exact 3:2 aspect preservation WIP).
  - XAVC HS 4K (HEVC) and XAVC S-I 4K All-Intra encoder tables still need remaining `libmpr.so` / `av-cam.bin` entries.

## Highlights & Key Files

- **Sensor / SGC (`fx30_open_gate/sources/fx30_ogsq_v6.c`, `scripts/generate_fx30_v6.py`):** Full 3:2 IMX684 readout (`5952x3968`), `CROPV = 96`, `Lrc_hscale` (`0x786`) kept at stock `5623`, `Lrc_vphase` (`0x78c`) = `3726`, `PDAF_OFFS` (`+14`), `PDAF_END_OFFS` (`+634`), `TG_Node10` (`0x30` == `S9`), `0x49c/0x4ac/0x4bc` (`+310`), `DrvM* vratio = 27/32`.
- **ISP `outsz` & LCD (`fx30_open_gate/sources/fx30_outsz_lcd_v1.c`):** ISP `outsz` (`0x0155d43c`, `0x0155d444`: `3840x2160 -> 3840x2560`) + VDF/AF table sites (`10 u32` writes).
- **Unified AP Helper (`scripts/build_fx30_ap_all_v4.py`, `fx30_open_gate/sources/fx30_ap_all_v4.c`):** Single zero-dependency AArch64 `.so` (`13.7 KB`) applying **all AP patches in 1 USB session** — V60 HAITA (`10` rules), V60 `RateCheck`, AVC Long-GOP + **XAVC S-I All-Intra** (`Row 9` + `105200/01/04/07`), and native `libXavcMp4` `avc1`/`tkhd`/`avcC` `3840x2560` headers (`0x21ff68`, `0x21fb34`, `0x2200a0`, `0x2263bc`, `0x180098`, `0x109768`). Runtime-verified on ARM64 Linux against FX30 6.01 `appFw.so` & `libmpr.so`.
- **Legacy AP Encoder (`scripts/build_fx30og_v3.py`):** Builds `libJit_level19_fx30og_v3.so` (`110` `av-cam.bin` patches + `7` `libmpr.so` patches).
- **V60 Card Unlock (`scripts/build_fx30_helpers.py`):** Builds `libJit_level19_fx30menu_v1.so` + `libJit_level19_fx30rate_v1.so`.
- **Steered DMM Contingency (`fx30_open_gate/sources/fx30_dmmwb_safe_v2.c`):** Safe `71 MiB` `0x6fa` allocation steering (`2x28.5 MB + 1 MB -> 0x8fe`).
- **MP4 Box Fixer (`scripts/fix_mp4_2560.py`):** Fixes `avc1`/`tkhd` boxes on recorded `.MP4` clips in place without re-encoding.
- **Runner (`fx30_open_gate/run_fx30_complete.py`, `fx30_open_gate/RUN_FX30_OPENGATE.command`):** `--verify-offline`, `--step1`, `--full`, `--full --with-dmmwb`, `--probe`, `--card-only`.
- **Upstream USB Runner Diff (`scripts/a7iv_runner_fx30.patch`):** Required CT1/CT6 patches (`6.01` firmware check, `fx30*` design SHA whitelist, `diskutil unmountDisk force` fallback) for `a7iv-open-gate-test`.

## Quick Usage

```bash
# 1. Verify all 8 compiled AP/CP binaries offline
python3 fx30_open_gate/run_fx30_complete.py --verify-offline

# 2. FAST 1-USB All-in-One: V60 + Open Gate 3:2 (AVC + All-I + native MP4 2560) + LCD (~5s, single USB session!)
sudo python3 -I -B fx30_open_gate/run_fx30_complete.py --fast

# 3. Legacy 4-USB flow (if needed)
sudo python3 -I -B fx30_open_gate/run_fx30_complete.py --full --with-dmmwb

# 4. Fix MP4 container avc1/tkhd metadata on older recorded clips (lossless)
python3 scripts/fix_mp4_2560.py /path/to/clips/
```
