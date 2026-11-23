# FX30 Open Gate — Dossiê Final Consolidado

> **Câmera:** Sony ILME-FX30, FW 6.01, sensor **IMX684** (APS-C 26 MP, 6240×4168 nativo 3:2)
> **Regra de ouro:** 100% volátil em RAM. Bateria fora 10 s = stock de fábrica. **Nunca** flash.
> **Data:** 23/11/2026

---

## 1. Veredito

**O Open Gate 3:2 em vídeo na FX30 NÃO foi concluído.** O sensor físico continua lendo 16:9 porque faltam os registradores **S0** (seleção de modo do sensor) e **S3** (timing de leitura) do IMX684.

**O que FUNCIONA 100%:** desbloqueio de cartão V60 (All-Intra 4K + 240 fps S&Q) e todo o mapa de memória da FX30.

---

## 2. O que foi conquistado (100% funcional)

| Item | Endereço / helper | Status |
|---|---|---|
| **Card Unlock V60** | `fx30menu_v1` + `fx30rate_v1` (CT6 level 19) | ✅ estável |
| **Dumper do CP** | `cp_dump.c` → zig + libsencore | ✅ reproduzível |
| **Dump do CP (33 MB)** | `fx30_cp_dump.bin` (0xC0000–0x2100000) | ✅ salvo |
| **outsz** (movie size table) | CP **`0x155d444`**: `000f7008` → `000f000a` | ✅ mapeado, aplicado, verificado |
| **dmmwb** (raw buffer DMM) | CP **`0x2080858/86c/880/970`**; size @`0x2080860/874` = `0x1788700`; key `0x57f`, grp `7` | ✅ mapeado |
| **S4** (geometry V offset) | CP **`0xc365d4`** = 502 → 0 | ✅ mapeado |
| **S2** (crop V offset) | CP **`0xc9b308`** = 502 → 0 | ✅ mapeado |
| **Mode table** | base CP `0xc76560`, passo `0x1770` (6000 B = L+R) | ✅ mapeado |
| **Mode 3** (vídeo 4K 16:9) | CP `0xc7abb0` (L) / `0xc7b768` (R) | ✅ |
| **Mode 10** (still 3:2) | CP `0xc84fc0` / `0xc85b78` | ✅ |
| **Mode 11** (vídeo 3:2 nativo!) | CP `0xc86730` / `0xc872e8` — w7=4168, w946=5, w947=8 | ✅ mapeado |
| **175 campos do record** | validados 174/175 contra o dump (idx 588 corrigido) | ✅ |

---

## 3. O mecanismo (o que destravou tudo)

### 3.1 O bug do path
O diretório do Jiritsu na câmera é **`/tmp_wbshd`** (underscore), não `/tmp/wbshd`.
E **`/tmp` fica na nflasha10 (persistente)** — cold boot NÃO limpa o `/tmp/jiritsu/libJit.so`.
Um `libJit.so` stale bloqueia o próximo teste (o CT6 recusa sobrescrever biblioteca de outro script).

### 3.2 O helper
```
zig cc -target aarch64-linux-gnu -std=gnu11 -O1 -fPIC -shared -nostdlib -ffreestanding \
  -fno-stack-protector -fno-builtin -fno-asynchronous-unwind-tables \
  -Wl,-soname,libJit.so -Wl,--no-undefined -Wl,-z,now -g0 -s \
  -Wl,-z,max-page-size=4096 helper.c <cam>/nflasha15/lib/libsencore.so -o libJit.so
```
Funções do `libsencore.so`:
- `_Z9dmm_allocjPj` / `_Z8dmm_freej` — buffer na RAM principal
- `sen_dma_transfer(dir, src, dst, size)` — `dir=0` CP→buffer (read), `dir=1` buffer→CP (write)

### 3.3 Aumentar o timeout USB
Em `ct1_common.py`, `GuardedDriver.read`: `timeout = 180000` quando `None` (dumps grandes).

---

## 4. Os 6 testes do Open Gate e por que falharam

| # | Abordagem | Resultado | Causa |
|---|---|---|---|
| 1 | Cópia bruta Mode 10 (still) | preta | clocks de streaming ausentes |
| 2 | 175 campos (diff Mode3→10, cru) | preta | 128 campos zerados mataram o vídeo |
| 3 | 175 campos + S2/S4 + S0(`0xc00b6c`) | preta | S0 errado |
| 4 | Mode 11 copiado (6000 B) | preta | **width mismatch** (Mode11=2108/canal vs Mode3=3192) |
| 5 | Height-Only (só w3/w7/w11/w943) | **vídeo 16:9 funciona!** | sensor não muda a física |
| 6 | Combo encoder+outsz+dmmwb | freeze/tally preso | sensor entrega 2160, ISP espera 2560 |

### A prova definitiva
Teste 5 gravou clipe **não-squeezed em 16:9** → o sensor continua fisicamente em 16:9.
O Height-Only altera só descritores de software. **Faltam S0/S3.**

---

## 5. O GAP: S0 e S3 do IMX684

### 5.1 A referência do A7 IV (`ogsq_v3.json`)
| Patch | Endereço A7 IV | O que faz |
|---|---|---|
| **S0** | `0xfb4044` | register-matrix column (sensor mode 16→2) |
| **S1** | `0xd95c42` | stream-0 active height (3942→4672) |
| **S2** | `0xc98476` | crop V offset (366→0) |
| **S3** | `0xd965fc` | **readout length (line periods)** 2011→2382 |
| **S4** | `0xc3dc92` | geometry row V offset (366→0) |
| **S5/S6** | `0xd95c3e/46` | window fields |
| **S7/S8** | `0xd96428/2c` | LC table |
| **S9** | `0xd965f4` | sensor reg 0x19 |
| **S10** | `0xd960b6` | aux/PDAF rows |

### 5.2 O que já foi procurado (e não existe na FX30)
- S3 estrutura `[2, 51, 6, X, 0, Y, 24, 1, 1]` → **0 ocorrências** no `cpapp-b.bin` e no dump
- S0 estrutura `02 03 04 05 06 06 07 08 08 09...` → a FX30 tem `0xc00c68`, `0xc30c78`, `0xc43b34` (sequências limpas `02 03 04 05 06 07...`), mas não a com duplicatas

### 5.3 ÂNCORAS DO RE (ponto de partida para a próxima sessão)

**Classes RTTI do IMX684** (achadas nesta sessão):
```
CP 0xd12060: typeinfo C_ImagerCtrl_IMX684E     (name string CP 0xc0fef0)
CP 0xd120a0: typeinfo ImagerDrvParam_IMX684E   (name string CP 0xc0ff20)
CP 0xd12070: tabela do ImagerDrvParam (NÃO é vtable Itanium padrão — entradas são dados)
```
**Classes do link do sensor** (file 0xb07da0, CP 0xc0fda0):
```
N6Camera6Imager17CetuSlvsecDriverE
N6Camera6Imager15CetuSlvsecLinkE
```

**O CP usa tabelas de descritor, não vtables.** O caminho é:
1. Achar o código que escreve no barramento do sensor (SPI/I2C do CP)
2. Ver quais campos do mode record viram escritas de registrador
3. Identificar S0 (modo) e S3 (timing)

### 5.4 Os 6 "stream heights" (candidato mais forte da rota RAM)

Cada mode record (L e R) tem um **sub-record** em `+0x740` com 3 campos de altura:

```
Left  sub-record @0xc7b2f0:
  0xc7b300: [4, 3164, 4, 4, 7, 8, 2, 3164]
Right sub-record @0xc7bea8:
  0xc7beb8: [4, 3164, 4, 4, 7, 8, 2, 3164]   ← estrutura IDÊNTICA
```

Endereços dos 6 campos (todos `3164` no stock):

| Lado | Endereço | Valor stock | Novo (Open Gate) |
|---|---|---|---|
| L | `0xc7b302` | 3164 | **4168** |
| L | `0xc7b30e` | 3164 | **4168** |
| L | `0xc7b32c` | 3164 | **4168** |
| R | `0xc7beba` | 3164 | **4168** |
| R | `0xc7bec6` | 3164 | **4168** |
| R | `0xc7bee4` | 3164 | **4168** |

**Atenção**: os endereços do Right são +2 em relação aos que aparecem em algumas notas
(`0xc7beb8`→`0xc7beba`, `0xc7bec4`→`0xc7bec6`, `0xc7bee2`→`0xc7bee4`).

**Observação do Mode 11**: o sub-record do Mode 11 nativo tem **686** (0x02ae) nesses
campos, não 4168 — então o sub-record não é simplesmente a "active height do header".

### 5.5 ARM32 **Thumb-2** — o CP não é ARM64

**Descoberta crítica:** o `cpapp-b.bin` é **ARM 32-bit Thumb-2** (`04109fe5` = `ldr r1,[pc,#4]` em ARM32, e os ponteiros de função têm o **bit Thumb** no LSB).

```python
md = Cs(CS_ARCH_ARM, CS_MODE_THUMB)   # ← o modo certo!
off = (va & ~1) - 0x108000             # ← limpar o bit Thumb
```

### Construtores e vtables (Thumb descompilado, validado)

```asm
; CP 0x8e6ce0 — ctor CetusSlvsecDriver: vtable 0xd10af4+8
; CP 0x8e77ca — ctor C_ImagerCtrl_IMX684:
;   str vtable(0xd10c44+8) no objeto
;   add.w r0, r4, #0x698      ← ImagerDrvParam fica no objeto+0x698
;   bl #0x8e87de              ← init dos params
```

### ImagerDrvParam — layout dos entries

```asm
; CP 0x8e87f2 — get_param(index):
;   ldr r2,[r3,#4]    ; count
;   ldr r0,[r3,#0x10] ; array base
;   mov.w r2,#0x160   ; 352 bytes por entry!
;   mla r0,r1,r2,r0   ; r0 = array + index*0x160
```

**As entradas do ImagerDrvParam têm 352 bytes** (`0x160`), não 3000.
A função de init `0x8e930c` itera as entries e chama a **vtable** (`[r0,#0x10]`) — a escrita no sensor.

### Próximo passo do RE

Descompilar `0x8e930c` (init) + as funções da vtable até achar a **escrita de registrador do sensor** (o S0 = mode, o S3 = timing).

### 5.6 A TABELA DE MODOS DO SENSOR (a descoberta chave)

A RE do driver (Thumb-2) levou aos objetos em RAM:

```
C_ImagerCtrl object : CP 0x13bd364  (vtable 0xd10c4c, modo ativo 0x8c0 em 0x13bd3ac)
ImagerDrvParam      : CP 0x13bd9fc  (vtable 0xd12078, count=31, array=0x20d93a4)
Array de 31 modos   : CP 0x20d93a4  (352 bytes por entry)
```

| Entry | Modo | Geometria |
|---|---|---|
| **6** | **0x8c0 (4K 24p)** | **5952×3348** (16:9) |
| 8, 18, 20-23, 27, 28 | vários | 3584 (16:9) |
| **25 / 26** | nativo | **6192×4128 (3:2 FULL!)** |

**As entries 25/26 são o modo 3:2 full-height nativo.** O sensor IMX684 **tem** o modo full.
O modo 4K (0x8c0) é a entry 6, com a geometria 16:9.

### 5.7 O PATCH (os dois helpers)

| Helper | O que faz |
|---|---|
| `sensor_geom_patch.so` (A) | 11 campos da entry 6 (5952×3348 → 6192×4128) + outsz (0x0870→0x0a00 em `0x155d446`) + mode ID 0x8c0 |
| `sensor_mode_copy.so` (B) | cópia integral da entry 25 sobre a entry 6 (geometria + timing 3:2) + mode ID + outsz |

Campos de geometria da entry 6 (todos verificados contra o dump):

```
0x20d9bf6  267 -> 270     (+0x12 seletor)
0x20d9c34  34  -> 12      (+0x50)
0x20d9c36  12  -> 26      (+0x52)
0x20d9c38  5952 -> 6192   (+0x54 LARGURA)
0x20d9c3a  12  -> 26      (+0x56)
0x20d9c3c  38  -> 8       (+0x58)
0x20d9c46  2   -> 8       (+0x62)
0x20d9c48  4   -> 20      (+0x64)
0x20d9c4a  3348 -> 4128   (+0x66 ALTURA)
0x20d9c4c  4   -> 20      (+0x68)
0x20d9c4e  2   -> 8       (+0x6a)
```

**Nota**: o outsz real é `0x155d446: 0x0870 -> 0x0a00` (os bytes são `00 0f 70 08` → `00 0f 00 0a`; o primeiro u16 é 0x0f00 e não muda).

### 5.8 Pista do 4K DCI

**Descoberta importante desta sessão:** o `cpapp-b.bin` é **ARM 32-bit** (`04109fe5` = `ldr r1,[pc,#4]` em ARM32), **não** ARM64.

Todos os disassembles anteriores estavam no modo errado (capstone ARM64 → instruções inválidas).

Para o RE:
```
md = Cs(CS_ARCH_ARM, CS_MODE_ARM)   # NÃO CS_ARCH_ARM64
```
O arquivo começa no vetor de reset (ARM32) e o código é 32-bit.

**Atenção ao offset**: `CP = file + 0x108000` foi validado para os DADOS (sites dos helpers). Para o código em endereços altos (`0x8exxxxx`), o mapeamento pode ser diferente — precisa ser confirmado com um descompilador de verdade (Ghidra com ARM32).

### 5.5 Pista do 4K DCI
A FX30 faz **4K DCI (4096×2160)** — prova que o pipeline não é engessado em 16:9.
E o **still 3:2 (6240×4168)** prova que o silício lê 4168 linhas.
O **Mode 11** é um modo de vídeo 3:2 nativo (w946=5, w947=8) — mas com largura 4.2K.
→ A rota "Caminho B" (usar o Mode 11 com o pipeline na largura dele) **não foi testada**.

---

## 6. Dados do dump (referência)

- `/proc/mounts` da FX30: `/tmp` na `nflasha10` (persistente), `/cp` na nflasha8, `/usr` na nflasha15
- CP = `cpapp-b.bin` file offset + `0x108000`
- `av-cam.bin` físico `0x401000000`
- Mode table: base `0xc76560`, passo `0x1770`
- DmmCfg.bin (nflasha8): keys `0x7d2c/0x7d40/0x7d54` + `0x7f88`

---

## 7. Segurança

- **Zero escrita em flash.** Todos os patches são RAM.
- O `.so` é gravado em `/tmp` (partição de rascunho) — benigno, não é firmware.
- **Recuperação**: bateria fora 10 s = stock.
- Se congelar/travar: bateria direto (não insistir no botão).

---

## 8. Próximos passos

1. **Card Unlock** — rodar `fx30menu_v1` + `fx30rate_v1` (CT6 level 19) para uso imediato
2. **RE do IMX684** — a partir das âncoras `0xd12060`/`0xd120a0` e `CetuSlvsecLink`
3. **Caminho B (não testado)** — Mode 11 nativo + pipeline na largura 4.2K

---

## 9. Artefatos

```
fx30_dump_helper/
  cp_dump.c / cp_dump_lib.so        dumper do CP
  outsz_write.c / outsz_lib.so      helper outsz
  fx30_cp_dump.bin                  dump 33 MB
  mode3_to_mode10_diff.txt          diff 198 linhas (128 zeros!)
  fx30_found_offsets.json
  FX30_FINDINGS_FOR_ANTIGRAVITY.md
  FX30_OGSQ_ADDENDUM_FOR_ANTIGRAVITY.md
  FX30_OGSQ_SYNC_ANTIGRAVITY.md
  FX30_OGSQ_MODE11_VERIFICATION_DOSSIER.md

a7siii-open-gate-test/              toolkit oficial (readcheck.c, build_s3.sh, PORT_NOTES.md)
a7iv-open-gate-test/                port original + helpers
```
