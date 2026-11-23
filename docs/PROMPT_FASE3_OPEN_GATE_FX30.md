# MISSÃO: FASE 3 — ENGENHARIA REVERSA AVANÇADA (S3 + RECORDS DE 3000 BYTES)
## OBJETIVO: OPEN GATE 3:2 NATIVO (3840x2560) NA SONY FX30 (FW 6.01)

Estamos iniciando a Fase 3 da Engenharia Reversa para ativar o Open Gate 3:2 na Sony FX30 (ILME-FX30, Firmware 6.01, BIONZ XR).
Todas as injeções são 100% voláteis em memória RAM via protocolo USB Jiritsu. Bateria removida por 10 segundos restaura o estado 100% de fábrica stock.

---

### 1. A REVELAÇÃO DEFINITIVA DO REPO OFICIAL DO A7 IV (`ogsq_v3.json`)

O arquivo funcional do A7 IV (`ogsq_v3.json`) tem exatamente **13 sites** (6024 bytes totais):
- **11 patches escalares (S0 a S10)**
- **2 RECORDS COMPLETOS DE 3000 BYTES (0xBB8)**:
  - Canal Left: `0xbf7638` (3000 bytes, com 180 campos modificados)
  - Canal Right: `0xbf81f0` (3000 bytes, `+0xBB8` à frente, com 180 campos modificados)

#### A Sacada de Engenharia do Autor no A7 IV:
- **Sensor Readout:** `[7032, 4688]` (o sensor lê a altura total física!)
- **Crop Ativo:** `[7008, 4672]`
- **Front-End Scaler:** `[4336, 2438]` (o primeiro estágio do scaler comprime a imagem verticalmente)
- **Size Source (ISP):** `[3840, 2160]` (o tamanho declarado de vídeo stock não quebra os pipelines secundários)

---

### 2. A PARIDADE ESTRUTURAL IDENTIFICADA NA FX30

A estrutura da FX30 é **100% idêntica** à do A7 IV, respeitando o offset de 3000 bytes:
- **Mode 3 Records na FX30 (Vídeo 4K):**
  - Canal Left:  CP `0xc7abb0` (3000 bytes / `0xBB8`)
  - Canal Right: CP `0xc7b768` (3000 bytes / `0xBB8`)
  - Diferença: `0xc7b768 - 0xc7abb0 = 0xBB8` (3000 bytes cravados!).

#### O Que Funcionou nas Fases 1 e 2:
1. **Card Unlock V60:** All-Intra 4K e 240 fps liberados e funcionais.
2. **Encoder AP (`fx30og_v2` / Level 19):** Tabelas `av-cam.bin` (21 sites) e `libmpr.so` (4 sites) gravando contêiner MP4 e XML em 3840x2560 sem rejeição.
3. **Display Driver (LCD Preview 3:2):** 7 sites validados (`0x1e2d3c` e `0xde791c..0xde7a10` na página `0x00de7000`).
4. **ISP Scaler & DMM Pool:** Outsz `0x155d446 = 0x0a00`, Buffers `0x2080860/74 = 29.29 MB`.
5. **Clock de Streaming Contínuo (Nó 0):** `0x01038d2b = 0x0f` (mantém o clock de vídeo ligado contínuo sem desarmar).

---

### 3. OS DOIS GAPS EXATOS PARA A FASE 3

1. **Localizar o S3 da FX30 (Temporizador Vertical de Readout):**
   - No A7 IV: `0xd965fc` (`0x07db` = 2011 -> `0x094e` = 2382 linhas de tempo).
   - Na FX30: Localizar onde vive esse "readout length / line periods" no subsistema do IMX684 (em torno dos Nós 40 a 55 em `0x01038d1c` ou `0x1123eXX`). É este temporizador que destrava a varredura vertical além de 3348 linhas sem congelar o preview.
2. **Adaptar os 180 Campos dos 2 Records de 3000 Bytes (Left e Right):**
   - Usar o array `old` e `new` de 3000 bytes do `ogsq_v3.json` (`0xbf7638` e `0xbf81f0`) como template.
   - Escalar os campos da FX30 em `0xc7abb0` (Left) e `0xc7b768` (Right) mapeando a altura de 3164 -> 4128/4168 linhas e ajustando a matriz interna de 180 campos (em vez de patchear apenas 10 campos isolados).

---

### 4. ARQUIVOS DISPONÍVEIS NO WORKSPACE

- Arquivo oficial de referência do A7 IV (13 sites completos com os dois records de 3000 bytes):
  `/Users/nauck/Documents/deepseek-harness/default-workspace/sony_research/a7iv-open-gate-test/windows/tools/modding/designs/ogsq_v3.json`
- DMM e LCD Designs:
  `/Users/nauck/Documents/deepseek-harness/default-workspace/sony_research/a7iv-open-gate-test/windows/tools/modding/designs/dmmwb4688_v1.json`
  `/Users/nauck/Documents/deepseek-harness/default-workspace/sony_research/a7iv-open-gate-test/windows/tools/modding/designs/outsz2560_v1.json`
  `/Users/nauck/Documents/deepseek-harness/default-workspace/sony_research/a7iv-open-gate-test/windows/tools/modding/designs/lcdfill_v1.json`
- Dump CP RAM FX30 (33.8 MB):
  `/Users/nauck/Documents/deepseek-harness/default-workspace/sony_research/fx30_dump_helper/fx30_cp_dump.bin`
- Binário descompactado do CP FX30:
  `/Users/nauck/Documents/sony_research/fwtool.py/unpacked_fx30/firmware.tar_unpacked/0700_part_image/dev/nflasha8_unpacked/cpapp-b.bin`
- Compilador:
  `python3 -m ziglang cc -target aarch64-linux-gnu -shared -fPIC -nostdlib -fno-builtin -fno-sanitize=all -O2`
- Script de envio Jiritsu:
  `/Users/nauck/Documents/deepseek-harness/default-workspace/sony_research/fx30_dump_helper/send_dump.py`
