# FX30 (ILME-FX30, FW 6.01) — Descobertas desta sessão (para o Antigravity)

> Tudo abaixo foi **descoberto/validado nesta sessão**. O objetivo continua sendo o
> **Open Gate 3:2 3840×2560** na FX30. Nada foi escrito em flash; tudo é RAM/volátil
> (bateria fora = stock).

---

## 0. Resumo executivo

| Tabela | Endereço CP | Status |
|---|---|---|
| **outsz** (Level 16, movie size table) | `0x155d444` | ✅ **ENCONTRADO + APLICADO + VERIFICADO na câmera** |
| **dmmwb** (Level 13, raw buffer DMM) | `0x2080858` / `0x208086c` / `0x2080880` / `0x2080970` | ✅ **ENCONTRADO** (falta o novo `size`) |
| **ogsq** (Level 5, geometria do sensor IMX684) | ? | ❌ **NÃO ENCONTRADO** |

**O que destrava tudo:** o mecanismo de leitura/escrita do CP via `libsencore.so` +
toolchain zig. Com ele o dump de 33 MB do CP foi obtido.

---

## 1. O BUG CRÍTICO do path (a causa de semanas de falha)

O diretório do Jiritsu na câmera é **`/tmp_wbshd`** (com **underscore**), **não**
`/tmp/wbshd` (com barra).

O `readcheck.c` oficial usa `#define RESULT_PATH "/tmp_wbshd/readcheck_result.txt"`.
O path relativo do Jiritsu (`../../tmp_wbshd/...`) resolve para `/tmp_wbshd/`.

**Todos os helpers custom falharam silenciosamente por causa disso.**
Todo `openat("/tmp/wbshd/...")` falhava → nenhum marcador, nenhum dump.

---

## 2. O mecanismo correto (dump/read do CP)

O helper é um `.so` carregado por `dlopen` no processo de serviço. Ele:

1. Aloca um buffer na RAM principal com `dmm_alloc`:
   - `extern u64 cam_dmm_alloc(u32 size, u32 *handle) __asm__("_Z9dmm_allocjPj");`
2. Abre `/dev/mem` e mapeia o buffer (físico).
3. Copia páginas do CP para o buffer com `sen_dma_transfer`:
   - `extern u32 sen_dma_transfer(u32 dir, u64 src, u64 dst, u32 size);`
   - `dir=0`: CP → buffer (**leitura**)
   - `dir=1`: buffer → CP (**escrita**)
   - `size` = 4096 (uma página)
4. Libera com `dmm_free` (`_Z8dmm_freej`).

O buffer físico fica em `0x4xxxxxxxx` (system RAM). Alinhamento 64 bytes.
Abrir `/dev/mem` com `O_RDONLY|O_SYNC` (leitura) ou `O_RDWR|O_SYNC` (escrita).
`mmap` em `phys - (phys & 0xfff)`, com `lead = phys & 0xfff`.

### Build (zig — o linker ELF que faltava)

```bash
python3 -m pip install --user ziglang   # versão usada: 0.17.1

python3 -m ziglang cc -target aarch64-linux-gnu -std=gnu11 -O1 \
  -fPIC -shared -nostdlib -ffreestanding \
  -fno-stack-protector -fno-builtin -fno-asynchronous-unwind-tables \
  -Wl,-soname,libJit.so -Wl,--no-undefined -Wl,-z,now -g0 -s \
  -Wl,-z,max-page-size=4096 \
  helper.c \
  /caminho/para/nflasha15/lib/libsencore.so \
  -o libJit.so
```

O `.so` resultante tem `DT_NEEDED = libsencore.so`, `SONAME = libJit.so`,
`.init_array` apontando para o construtor. **É esse formato que o loader da FX30
aceita** — helpers zig compilados assim **rodam** (provado).

### Protocolo USB (já existente, do CT6)

- `SET_FILE_PATH = 0x400F`, `SET_FILE2 = 0x4010`, `GET_FILE2 = 0x4011`,
  `FILE_DELETE2 = 0x4012`, categoria `0x1001`.
- Lib path: `/tmp/jiritsu/libJit.so` (exato).
- Result path: `/tmp_wbshd/readcheck_result.txt` (ou outro `/tmp_wbshd/*`).
- **Aumentar o timeout de leitura do USB** para dumps grandes: em
  `ct1_common.py`, `GuardedDriver.read`, use `timeout = 180000` quando `None`.

---

## 3. outsz (Level 16) — ENCONTRADO E APLICADO

### Endereço e patch

```
CP 0x155d444   (dentro da página 0x155d000)
old: 00 0f 70 08   (0x08700f00 = 3840×2160)
new: 00 0f 00 0a   (0x0a000f00 = 3840×2560)
```

### Como foi identificado

A movie size table da FX30 tem a **mesma estrutura e as mesmas unidades** do A7 IV:

```
0x155d43c: 000f7008 3e040000   ← 1º entry: 3840×2160, unit 0x43e
0x155d444: 000f7008 d9020000   ← 2º entry: 3840×2160, unit 0x2d9  ← SITE
0x155d44c: ... 00083804 ...    ← 2048×1080
```

O site do A7 IV (`0x143c7bc`) é o "segundo entry" (unit `0x2d9`). O da FX30 é
`0x155d444`. **Mesmas units (0x43e/0x2d9) = mesma tabela.**

### Resultado na câmera (IMPORTANTE)

O helper `outsz_lib.so` foi aplicado e **verificado** (`osz_ok = OK`, leitura de
volta confirmando `000f000a`).

**PORÉM: aplicar o outsz SOZINHO CONGELA a câmera** (tally light travado aceso,
freeze; recuperado tirando a bateria). Isso **prova** que:
- o endereço está **correto** (o CP reagiu ao valor novo);
- o write via `sen_dma_transfer(1,...)` **funciona**;
- o outsz **não pode ir sozinho** (falta o buffer maior e/ou o sensor 3:2).

---

## 4. dmmwb (Level 13) — ENCONTRADO

### Endereços e valores

```
CP 0x2080858: grp=7 key=0x57f size=0x1788700 (24.676.096) cnt=0x10f z=0
CP 0x208086c: idem
CP 0x2080880: idem
CP 0x2080970: idem  (separado; os 3 primeiros são contíguos, 0x14 apart)
```

Estrutura de record = `[u32 grp][u32 key][u32 size][u32 cnt][u32 0]` (20 bytes).

Bate com o `DmmCfg.bin` estático da FX30
(`nflasha8_unpacked/DmmCfg.bin`, keys em `0x7d2c/0x7d40/0x7d54` + `0x7f88`,
size `0x1788700`).

### O que falta

O **novo `size`** do buffer para o open gate 3:2. Depende da **altura de leitura
do sensor** (que viria do ogsq). No A7 IV: `0x1a92d00` → `0x1f79800`
(= 4688 × 7040). O novo size da FX30 = (linhas do open gate) × (stride).

Tentativas de fatorar `0x1788700`: não divide limpo por 6240/7008/7040/3510/4168
— pode ter header/overhead.

---

## 5. ogsq (Level 5) — NÃO ENCONTRADO (o que falta)

### Fatos

- Sensor da FX30 = **IMX684** (classe `ImagerDrvParam_IMX684E`, CP `0xc0ff00`).
  (NÃO é IMX571.)
- Os endereços do ogsq do A7 IV (IMX710) **não mapeiam**:
  - mode records `0xbf7638` / `0xbf81f0` → zeros/outra coisa
  - driver params `0xc3dc92`, `0xc98476`, `0xc8dc3e` → dados diferentes
  - stored records A7 IV/A7S III → dados diferentes

### O que já foi tentado no dump de 33 MB (COM modo Filme ativo)

| Busca | Resultado |
|---|---|
| `6240` + `4168/4160` na região do driver | 0 candidatos |
| `6240` + crop heights (`3510/3584/2340`) no dump | 120 hits, **todos código ARM** |
| clusters de `4168`/`4160` | código |
| `6240×3510` / `6240×3512` em 0x40 | **0 hits** |
| `4160×2340` em 0x40 | 2 hits (`0x1c380d3`, `0x203d1b2`) → dados de alta entropia |
| `6240×4168` em 0x40 | 2 hits (`0x9b5dc0`, `0x203d1b2`) → dados de alta entropia |
| padrão `[X,Y,0x50,0x1a,X2,Y2,0x5c,0x22]` | 0 hits |
| região `0x11a3000` (camera core do A7 IV) | **zerada** |
| shape: 10+ u16 em 3450–3600 na static | 124 janelas, **todas código** |

### Descoberta parcial: região do driver do sensor

A região do driver do sensor na FX30 também fica em **CP `0xc3xxxx`–`0xc4xxxx`**
(mesma faixa do A7 IV), MAS a estrutura é diferente:

```
A7 IV @0xc3dc80 (u16): 0000 0000 0302 016e 08c0 0000 ...   ← pares (reg, val)
FX30  @0xc3dc00 (u32): 0x00600d40 0x00600d41 0x00600d42 ... ← [0x0060, valor]
```

Os valores da FX30 (`0x00600d40`, `0x00600f55`, ...) parecem ser uma tabela de
valores do sensor (não pares reg/val como no A7 IV). **Precisa de RE para
interpretar.**

### Conclusão

O IMX684 **não guarda o mode record** na estrutura u16 do IMX710. Pode ser
comprimido/outro formato, ou estar em região não mapeada. Sem o ogsq, o buffer
novo (dmmwb) não fecha.

---

## 6. Fatos técnicos verificados

- `CP_runtime_address = cpapp-b.bin_file_offset + 0x108000` (inverse: `file = CP − 0x108000`).
  Validado na A7 IV (lcdfill + 11 patches ogsq).
- `av-cam.bin` = tabelas do encoder + ORIL (físico `0x401000000`).
- `DmmCfg.bin` (nflasha8) = fonte estática do dmmwb; `0800_appli/setting/DmmCfg.bin`
  (66 KB) **não tem** as keys `0x57f` (é outro config).
- Dump completo do CP: `0xC0000`–`0x2100000` (33 MB).
- `/proc/mounts` da FX30: `/tmp_wbshd` = tmpfs, `/cp` = nflasha8, `/usr` = nflasha15.
- Helpers do Gemini que já funcionam na FX30: `fx30menu_v1`, `fx30rate_v1`,
  `fx30og_v2`, `fx30lcd_v2` (todos *modificações* dos helpers A7 IV do autor).

---

## 7. Artefatos gerados (em `fx30_dump_helper/`)

| Arquivo | O que é |
|---|---|
| `cp_dump.c` / `cp_dump_lib.so` | dumper do CP (33 MB) — **funciona** |
| `outsz_write.c` / `outsz_lib.so` | helper do outsz (`0x155d444`) — **aplicado e verificado** |
| `fx30_cp_dump.bin` | dump de 33 MB do CP |
| `dump_1.5MB_0x1300000.bin` | dump parcial inicial |
| `fx30_found_offsets.json` | resumo dos offsets |
| `send_dump.py` | script de envio/leitura (paths `/tmp_wbshd/...`) |
| `test_s3.py` | teste com o dumper oficial do A7S III (que **rodou** na FX30) |

Repos:
- `a7siii-open-gate-test` (ArcaneLantern) — **tem o toolkit completo**:
  `readcheck.c`, `s3read.c`, `build_s3.sh` (zig), `gen_s3.py`, `PORT_NOTES.md`,
  `libJit_s3_dump.so`. **Foi o que revelou o mecanismo.**
- `a7iv-open-gate-test` (ArcaneLantern) — o port original.

---

## 8. O que falta (para o Antigravity focar)

1. **ogsq da FX30 (IMX684)**: achar os mode records / a geometria 3:2.
   - Sugestão: RE do código que lê o mode record → seguir ponteiro.
   - A região do driver está em `CP 0xc3xxxx–0xc4xxxx` (estrutura `[0x0060, valor]`).
2. **Novo `size` do dmmwb**: precisa da altura de leitura do open gate (do ogsq).
3. **Pacote final**: aplicar **outsz + dmmwb + ogsq juntos** (o outsz sozinho
   congela).
4. **Verificar se a FX30 já tem um modo vídeo 3:2 nativo** (o A7S III resolveu
   assim: apontou o 4K pro modo full-height existente `0xe011`). Se a FX30 tiver
   um modo foto 3:2, dá pra apontar o 4K pra ele.

---

## 9. Procedimento de teste seguro

- Só RAM. Bateria fora = stock. **Nunca** escrever em flash.
- O helper de leitura (`cp_dump_lib.so`) é **read-only** (`sen_dma_transfer(0,...)`).
- O helper de escrita (`outsz_lib.so`) verifica os bytes stock antes de escrever e
  lê de volta.
- Para recuperar de freeze: tirar a bateria por 10 s.

---

## 10. Comandos úteis

```bash
# dump do CP (33 MB) — atualizar START/END em cp_dump.c
python3 -m ziglang cc -target aarch64-linux-gnu -std=gnu11 -O1 -fPIC -shared \
  -nostdlib -ffreestanding -fno-stack-protector -fno-builtin \
  -fno-asynchronous-unwind-tables -Wl,-soname,libJit.so -Wl,--no-undefined \
  -Wl,-z,now -g0 -s -Wl,-z,max-page-size=4096 cp_dump.c \
  <FX30>/nflasha15/lib/libsencore.so -o cp_dump_lib.so
```

Rodar (macOS, com sudo para takeover do USB):
```bash
echo 'SENHA' | sudo -S -k /bin/bash /tmp/run_dump_root.sh
```
