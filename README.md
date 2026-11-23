# Sony FX30 (ILME-FX30, FW 6.01) — V60 Card Unlock & Open Gate 3:2 Research

Volatile RAM-only reverse engineering research for the **Sony ILME-FX30 (Firmware 6.01, IMX684 / BIONZ XR)** via USB Service Mode (Jiritsu). **Zero NAND/flash writes** — removing the battery for 10 seconds restores 100% stock behavior.

> ⚠️ **Disclaimer:** Use strictly at your own risk on hardware you own. No proprietary Sony firmware, dumps, or compiled binaries are included in this repository.

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

## Key FX30 Discoveries (FW 6.01)

1. **V60 Card Unlock:**
   - 10 HAITA rules in `appFw.so` (`0x047c1eb0 .. 0x047c1f40`, rules `0x02a6`–`0x02af`) flipped from `0x05` to `0x00`.
   - `RateCheck::GetMediaTransferRateInh` in `libmpr.so` (`0x103c88`) patched to return `0`.
2. **FX30 vs. A7 IV Structural Difference:**
   - Unlike the A7 IV (`7008px` in both photo and video), the FX30 reads `6192x4128` in 3:2 still mode but crops horizontally to `5952x3348` in 4K 16:9 video (`1.55x` exact oversampling).
   - Video DMA line pitch is locked at `5952 px`; exceeding it causes a hardware AXI bus fault. Target Open Gate geometry is **`5952 x 3968`** (`3968 / 1.55 = 2560`).
3. **Jiritsu Persistence Quirk on FX30:**
   - `/tmp` resides on `nflasha10` (persistent across cold boots), requiring `FILE_DELETE2` on `/tmp/jiritsu/libJit.so` before each injection.
   - Debug markers must be written under `/tmp_wbshd/` (with underscore).

---

## Repository Layout

- `scripts/build_fx30_helpers.py` — Builds `fx30menu_v1`, `fx30rate_v1`, `fx30lcd_v2`, and `fx30og_v2` helper binaries from upstream templates.
- `fx30_open_gate/` — Volatile CP DMA C helpers, headers, and Python/shell USB injection runners.
- `docs/` — Mapped offsets JSONs (`fx30_card_unlock_v1.json`, `fx30_mode3_180_fields_layout.json`) and research reports.

---

## Credits & License

- Built upon the volatile USB Jiritsu / DMA injection architecture by **[ArcaneLantern](https://github.com/ArcaneLantern/a7iv-open-gate-test)**.
- Uses Sony USB service mode & firmware unpacking foundations from **[Sony-PMCA-RE](https://github.com/ma1co/Sony-PMCA-RE)** and **[fwtool.py](https://github.com/DavidBuchanan314/fwtool.py)**.
- Licensed under **GNU GPL-3.0** (see `LICENSE`).
