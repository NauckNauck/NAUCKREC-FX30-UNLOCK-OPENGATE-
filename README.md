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
| **Open Gate 3:2 (`5952x3968 -> 3840x2560`)** | 🔴 **Not achieved (24 tests)** | The sensor physically streams (live image + one 8 s clip were produced from column 11), but no configuration has yet delivered 3968 lines to the ISP. Full test-by-test record: `fx30_open_gate/docs/FX30_TEST_REGISTRY.md`. |

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

```
fx30_open_gate/
  fx30_open_gate.c          original combo helper (Mode 10 / 175-field attempts)
  fx30_full_opengate.c      393-patch combo
  fx30_og_phase3.c          Mode 2 record phase (3/3b/3c/3d lineage)
  fx30_pure_a7iv.c          A7 IV-style scalars only (no TG writes)
  mode10_data.h / mode11_data.h / patch_data.h / records_180_data.h
  sources/                  ALL 29 single-purpose CP helpers, one per test:
    col11_mode2.c           column 11 + Mode 2 records
    dci_native.c            native DCI stretched 3140 -> 3968   (last attempt)
    dci_col9.c/.v2          DCI via column-9 selector
    final_sb.c              node0 + S0 + entry 25   (produced the 8 s clip)
    height_only.c           A7 IV scalars, no column swap
    opengate_path_a.c       physical TG registers + 3968
    opengate_phase2.c       sensor + ISP + dmmwb + LCD
    sensor_geom_patch.c     entry 6 (11 geometry fields) + outsz
    sensor_mode_copy.c      full entry 25 copy
    sensor_only.c           entry 6 without outsz
    strategyB_s0.c          entry 25 + S0
    two_axis_og.c           column 11 + BOTH axes (5952->6192)
    phase3.c/3b/3c/3d       Mode 2 record evolutions
    pure_a7iv.c             360 record fields, no TG
    full_ogsq.c / opengate_video_master.c / outsz_write.c / cp_dump.c / dump.c / stub.c
  data/                     all_154_nodes.txt, mode3_to_mode10_diff.txt,
                            fx30_found_offsets.json, fx30_mode3_180_fields_layout.json
  docs/                     FX30_COMPLETE_SYNTHESIS.md   (full RAM map, 11 parts)
                            FX30_TEST_REGISTRY.md       (24 tests, what failed and why)
                            FX30_FINAL_DOSSIER.md, FX30_OPEN_GATE_RELATORIO_COMPLETO.md,
                            GEMINI_PROMPT_OPEN_GATE_FINAL.md and the Antigravity dossiers
scripts/
  build_fx30_helpers.py     builds the level-19/16 helpers (menu, rate, lcd, encoder)
  send_dump.py              Jiritsu injector - pick the helper with FX30_HELPER=<name.so>
  diagnose.py diff_map.py mkelf.py ogsq_verify.py test_*.py
logs/
  fx30_injection_sessions.log   68 consolidated injection sessions (2 MB)
```

### Building a helper

```bash
SEN=<path>/libsencore.so
python3 -m ziglang cc -target aarch64-linux-gnu -std=gnu11 -O1 -fPIC -shared -nostdlib \
  -ffreestanding -fno-stack-protector -fno-builtin -fno-asynchronous-unwind-tables \
  -Wl,-soname,libJit.so -Wl,--no-undefined -Wl,-z,now -g0 -s -Wl,-z,max-page-size=4096 \
  fx30_open_gate/sources/dci_native.c "$SEN" -o fx30_open_gate/sources/dci_native.so
```

### Injecting

```bash
# camera in Mass Storage (054C:0E0E)
FX30_HELPER=fx30_open_gate/sources/dci_native.so python3 scripts/send_dump.py
```

---

## Credits & License

- Built upon the volatile USB Jiritsu / DMA injection architecture by **[ArcaneLantern](https://github.com/ArcaneLantern/a7iv-open-gate-test)**.
- Uses Sony USB service mode & firmware unpacking foundations from **[Sony-PMCA-RE](https://github.com/ma1co/Sony-PMCA-RE)** and **[fwtool.py](https://github.com/DavidBuchanan314/fwtool.py)**.
- Licensed under **GNU GPL-3.0** (see `LICENSE`).
