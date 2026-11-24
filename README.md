# Sony FX30 (ILME-FX30, FW 6.01) — V60 Card Unlock & Open Gate 3:2 Research

Volatile RAM-only reverse engineering research for the **Sony ILME-FX30 (Firmware 6.01, IMX684 / BIONZ XR)** via USB Service Mode (Jiritsu). **Zero NAND/flash writes** — removing the battery for 10 seconds restores 100% stock behavior.

> ⚠️ **Disclaimer:** Independent community research with no affiliation, endorsement, or warranty from Sony. Use strictly at your own risk on hardware you own. No proprietary Sony firmware, dumps, encryption keys, or compiled binaries are included in this repository.

---

## Current Status

| Feature | Status | Details |
| :--- | :---: | :--- |
| **SD V60 Card Unlock (`fx30menu_v1` + `fx30rate_v1`)** | 🟢 **Working** | Unlocks **XAVC S-I 4K (All-Intra)** and **S&Q 240 fps** in menus and bypasses `RateCheck::GetMediaTransferRateInh` so V60 SD cards record stably without requiring V90 or CFexpress Type A. |
| **Encoder AP (`fx30og_v2`)** | 🟢 **Validated** | Patches 21 sites in `av-cam.bin` and 4 sites in `libmpr.so` to accept `3840x2560` (AVC & HEVC 10-bit 4:2:2). |
| **LCD 3:2 Preview (`fx30lcd_v2`)** | 🟢 **Validated** | Patches 7 display sites (`0x1e2d3c`, `0xde791c..0xde7a10`) for full 3:2 LCD live view without 16:9 letterbox bars. |
| **CP Memory Map & IMX684 Timing** | 🟢 **Mapped** | Mapped `C_ImagerCtrl_IMX684`, 31-mode `ImagerDrvParam` table (`0x20d93a4`), 154-node matrix (`0x01038d1c`), P2S buffer (`0x01123xxx`), `Mode 2` SGC record (`0xc79440` L / `0xc79ff8` R), `outsz` (`0x155d446`), and DMM pool (`0x2080860/74`). |
| **Open Gate 3:2 (`5952x3968 -> 3840x2560`)** | 🟡 **In Progress** | Physical sensor streaming proved possible; ongoing work aligns Mode 2 180-field SGC scaling + `0x8e8218` sensor re-init without bus contention or horizontal DMA pitch overflow. |

---

## Prior Work vs. Original FX30 Discoveries

To keep attribution clear and transparent, below is the exact separation between upstream community foundations (for other Sony cameras) and what was independently reverse-engineered for the **Sony FX30 (FW 6.01)**.

### 1. Prior Work by the Community (Other Cameras / Foundations)
- **[ArcaneLantern (`a7iv-open-gate-test` / `a7siii-open-gate-test`)](https://github.com/ArcaneLantern):**
  - Created the volatile USB Service Mode (Jiritsu / `libJit.so`) injection workflow and AP-to-CP DMA bridge (`cam_dmm_alloc` + `sen_dma_transfer`).
  - Proved and released working **Open Gate 3:2 (`3840x2560` & `5760x3840`) on the Sony A7 IV** (`ogsq_v3`, `outsz2560_v1`, `dmmwb4688_v1`, `lcdfill_v2`) and cropped Open Gate on the A7S I.
- **[Erlla (`sony-a7m4-custom-firmware`)](https://github.com/Erlla/sony-a7m4-custom-firmware):**
  - Developed persistent custom firmware tooling (Fastboot/RP2040 and no-Fastboot installer) for the **Sony A7 IV (FW 6.02)**, covering FL2/FL3 color profiles, custom LUTs, photo aspect ratios, custom shutter/focus sounds, third-party battery unlock, and shell/SSH access. *(No Open Gate video or FX30 support.)*
- **[OlliV (`fx30_fw`)](https://github.com/OlliV/fx30_fw):**
  - Surveyed FX30 FW 6.01 on the AP side (`appFw.so` & BK4 settings container), documenting `PRODUCT_MODEL_LAX`, HAITA/BKID menu gating, Pixel Shift testing, and XAVC HS 8K limits.
- **[ma1co (`Sony-PMCA-RE`)](https://github.com/ma1co/Sony-PMCA-RE) & [DavidBuchanan314 (`fwtool.py`)](https://github.com/DavidBuchanan314/fwtool.py):**
  - Foundational Sony USB Senser/Service Mode protocol and FDAT firmware decryption/unpacking tools.

### 2. Original Discoveries in This Project (Sony FX30 / FW 6.01 / IMX684)
- **Complete SD V60 Card Unlock (`fx30menu_v1` + `fx30rate_v1`) — 100% Working:**
  - Located and patched the 10 HAITA rules in FX30 `appFw.so` (`0x047c1eb0 .. 0x047c1f40`, rules `0x02a6`–`0x02af`) from `0x05` to `0x00` to unlock XAVC S-I 4K and S&Q 240 fps menus.
  - Located and patched `RateCheck::GetMediaTransferRateInh` in FX30 `libmpr.so` (`0x103c88`) to bypass the artificial V90/CFexpress speed gate on V60 cards.
- **Full FX30 6.01 Port of Encoder, DMM & LCD 3:2 Patches:**
  - Mapped the 21 sites in `av-cam.bin` and 4 sites in `libmpr.so` (`fx30og_v2`) for `3840x2560` AVC & HEVC 10-bit 4:2:2 recording.
  - Mapped the 7 FX30 LCD driver sites (`fx30lcd_v2`: `0x1e2d3c`, `0xde791c..0xde7a10`), ISP `outsz` (`0x155d446`), and DMM pool (`0x2080860`/`0x2080874` -> `29.29 MB`).
- **Coprocessor (CP ARM32 Thumb-2) & IMX684 Sensor Reverse Engineering:**
  - Extracted a 33.8 MB live CP RAM dump (`0x000c0000`–`0x02100000`) and mapped `CP = file_offset + 0x108000` in `cpapp-b.bin`.
  - Mapped `C_ImagerCtrl_IMX684`, the 31-mode `ImagerDrvParam` table (`0x20d93a4`), the 154-node register matrix (`0x01038d1c`), the P2S buffer (`0x01123xxx`), routine `0x8e8218` (VTable[64]), and the IMX684 Timing Generator formula (`Active Half-Lines = H/2 + 6`, `VMAX = 2167`).
  - Identified that 4K 24p consumes **Mode 2 SGC (`0xc79440` L / `0xc79ff8` R)** rather than Mode 3.
- **FX30 vs. A7 IV Structural Discovery:**
  - Proved why A7 IV patches do not port 1:1 to the FX30: while the A7 IV uses full sensor width (`7008 px`) in both photo and video, the FX30 reads `6192x4128` in 3:2 still mode and crops horizontally to `5952x3348` (`1.55x` oversampling) in 4K video, with DMA line pitch locked to `5952 px` (making `5952x3968` the required Open Gate readout target).
- **FX30 Jiritsu Operational Quirks:**
  - Discovered that `/tmp` resides on `nflasha10` (persistent across cold boots), requiring `FILE_DELETE2` on `/tmp/jiritsu/libJit.so` before every injection, and that debug markers must be written under `/tmp_wbshd/` (with underscore).

---

## Repository Layout

- `scripts/build_fx30_helpers.py` — Builds `fx30menu_v1`, `fx30rate_v1`, `fx30lcd_v2`, and `fx30og_v2` helper binaries from upstream templates.
- `fx30_open_gate/` — Volatile CP DMA C helpers, headers, and Python/shell USB injection runners.
- `docs/` — Mapped offsets JSONs (`fx30_card_unlock_v1.json`, `fx30_mode3_180_fields_layout.json`) and research reports.

---

## License

Licensed under **GNU GPL-3.0** (see `LICENSE`).
