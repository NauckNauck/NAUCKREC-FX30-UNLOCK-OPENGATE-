# Auditoria Técnica e Status Atual — Sony FX30 (FW 6.01) Open Gate 3:2

**Última atualização:** 2026-11-25

---

## 1. O Que Já Está Comprovado em Hardware

- **Card Unlock V60 (`fx30menu_v1` + `fx30rate_v1`):**
  - 10 regras HAITA em `appFw.so` (`0x047c1eb0..0x047c1f40`, IDs `0x02a6..0x02af`, `0x05 -> 0x00`) + `RateCheck::GetMediaTransferRateInh` (`0x103c88` em `libmpr.so`).
  - Destrava **All-Intra 4K (XAVC S-I)** e **S&Q até 240 fps** em cartões SD V60.
- **Leitura Full 3:2 no Sensor IMX684 (`fx30_ogsq_v6.c`):**
  - `480 u16 + 17 u8` patches validados byte a byte contra `fx30_cp_dump.bin` (`0 mismatches`, `<= 32` páginas CP).
  - Mode 2 SGC (`0xc79440` L / `0xc79ff8` R, 180×2 campos): `CROPV = 96`, `Lrc_hscale` (`+0x786`) mantido em `5623` stock (elimina listras verticais), `Lrc_vphase` (`+0x78c`) = `3726`, `PDAF_OFFS` (`+14`), `PDAF_END_OFFS` (`+634`), `0x49c/4ac/4bc` (`+310`), `TG_Node10 = 0x30` (`48 == S9`), `S10 = 82`, `Node7 = 2`, `S5/S6 = 2` stock, `DrvM* vratio = 27/32`.
- **Gravação Nativa `3840x2560` 4:2:2 10-bit em XAVC S 4K:**
  - Comprovada diretamente no bitstream H.264/SPS (`coded=3840x2560`, `crop=(0,0,0,0)`) e reprodução visual (`TEST_0057_3840x2560.MP4`), cores corretas e zero pixels perdidos.
- **Corretor Lossless de Container (`scripts/fix_mp4_2560.py`):**
  - Corrige os boxes `avc1` e `tkhd` (`3840x2160 -> 3840x2560`) no `.MP4` sem reencodar.

---

## 2. Estado do Build Atual & Pendências Mapeadas

### A. Container MP4 Nativo (`avc1` / `tkhd` / `avcC`) em `libmpr.so`
- `scripts/build_fx30og_v3.py` (`112` records em `av-cam.bin`: `2` guards + `110` patches; `9` records em `libmpr.so`: `2` guards + `7` patches, SHA-256 `1b63fdff...`) já inclui `0x1164B0` (`XavcsMovieWriter`), `0x180098` (`TransMPFResolution2PubDefinition`) e `0x109768` (`xavcsMovieMeta::WriteMetaInfo`). Ele foi compilado e injetado em câmera (`CT6_20261125T080504Z_level19_run_fx30og_v3`), aguardando teste de gravação após recarga da bateria.
- Disassembly adicional de `libmpr.so` mostrou que os literais hardcoded de altura `2160` (`#0x870`) do muxer `libXavcMp4` ficam em:
  - **`0x21ff68`** (`0x21fef0` Track Setup — case `"avc1"`: `mov w6, #0x870` gravado em `[x0+0x40]`)
  - **`0x2200a0`** (case `"hvc1"`: `mov w6, #0x870`)
  - **`0x21fb34`** (`0x21fafc`, checagem de `level_idc` em `avcC`: `cmp w0, #0x870`; sem atualizar para `#0x0a00`, cai em Level 4.0 `0x28`)
  - **`0x2263bc`** (`0x22632c`, segundo caminho com `"avc1"` + `#0x870`)

### B. Preview LCD 3:2 sem Esticamento Vertical (`fx30_outsz_lcd_v1.c`)
- Diferente da A7 IV (painel LCD 3:2 `panel_code = 1` em `0x001e2d50`), o painel físico LCD da FX30 (`panel_code = 2` / grupo 2 `idx 10..14` em `0x00de78dc..0x00de79cc`, canvas `640x480` com 3:2 = `(0, 26, 640, 428)`) é **4:3**.
- Forçar `fit_mode = 0` (`fill`) em `0x001e2d38` preenche `4:3` com imagem `3:2`, causando **+12,5% de esticamento vertical**, e os retângulos AF/Tracking da `fx30_outsz_lcd_v1.c` alteram `idx 8` (grupo 1) em vez do painel `4:3`.
- **Próximo passo:** manter proporção preservada (`fit_mode = 3` ou corrigir a razão de aspecto entregue pelo modo Filme em `0x66187e`/`0x1e38e0` de `16:9` para `3:2`) + alinhar os retângulos AF ao grupo correto.

### C. XAVC HS 4K (HEVC) & XAVC S-I 4K (All-Intra)
- `libmpr.so` possui **5** linhas `3840x2160` em `mpr::g_videoParamComb` (`0x2cb138`) e `mpr::pconv::m_vpcSt` (`0x2e0bb0`), das quais `fx30og_v3` altera apenas as 2 linhas AVC Long-GOP (`Rows 0 e 2`), deixando:
  - **HEVC (XAVC HS 4K):** `Rows 5 e 7` (`0x2cb1b6`, `0x2cb1e6`) + `m_vpcSt` correspondentes + descritores HEVC em `av-cam.bin` ainda em `2160` (causando reboot imediato ao gravar em XAVC HS).
  - **XAVC S-I (All-Intra 4K):** `Row 9` (`0x2cb216`, `0x2e0cb2`) + descritores `105200`/`105201` em `av-cam.bin` ainda em `2160`.
- **Próximo passo:** construir um helper AP próprio em C (`zig cc`) sem o limite de 9 registros da Table-2 do `.so` herdado da A7 IV, unificando V60 + Open Gate (AVC + HEVC + All-I + muxer MP4) em 1 única etapa AP.
