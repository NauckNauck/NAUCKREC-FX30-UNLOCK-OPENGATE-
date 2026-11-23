# FX30 Open Gate — Addendum: OGSQ / Driver do Sensor (S0–S10)

> Complemento ao `FX30_FINDINGS_FOR_ANTIGRAVITY.md`.
> Foco: o que já foi mapeado do **ogsq** e o que ainda falta do **driver físico do sensor**.

---

## 1. State of play

| Peça | Status |
|---|---|
| **outsz** `0x155d444` | ✅ aplicado e verificado (sozinho congela) |
| **dmmwb** `0x2080858` etc. | ✅ aplicado pelo Antigravity (size `0x01bf0000`, 29.29 MB) |
| **ogsq — Mode 3 record** `0xc7abb0` (L) / `0xc7b768` (R) | ✅ patch cirúrgico de 198 campos aplicado — **tela preta** |
| **ogsq — driver físico (S0/S3/S4)** | ❌ ainda falta |

O patch cirúrgico do Mode 3 (198 campos de altura) **não acende** a imagem:
o sensor físico continua emitindo o bloco 16:9, e o scaler fica esperando as
linhas que não chegam (line-count mismatch → black screen).

---

## 2. Mode Records do IMX684 (confirmados no dump de 33 MB)

```
Modo 3  (vídeo 4K 16:9 stock):  L @ CP 0xc7abb0   R @ CP 0xc7b768   (0xBB8 apart)
Modo 10 (still 3:2 full height): L @ CP 0xc84fc0   R @ CP 0xc85b78
Tabela base:                       CP 0xc76560
```

Cada record = **3000 bytes** (0xBB8), par L/R = 6000 bytes por modo.

### Alturas-chave (Mode 3 → Mode 10)

| Offset no record | Mode 3 (16:9) | Mode 10 (3:2) | Significado |
|---|---|---|---|
| `+0x006` | 3190 (0x0c76) | **4200** (0x1068) | raw/crop end |
| `+0x00e` | 3164 (0x0c5c) | **4168** (0x1048) | **altura ativa** |
| `+0x016` | 3156 (0x0c54) | **4128** (0x1020) | altura efetiva |
| `+0x02a` / `+0x032` / `+0x04e` / `+0x056` / `+0x066` / `+0x06e` / `+0x092` | 3164 | **4168** | repetições da matriz |
| `+0x03a` / `+0x05e` / `+0x076` / `+0x09a` | 3156 | **4128** | repetições |

**O diff completo L (198 linhas) está em** `mode3_to_mode10_diff.txt`
(colunas: `idx / mode3 / mode10`).

---

## 3. DrvParam da FX30 — ENCONTRADO em `CP 0xc7b30c`

Estrutura (u16):

```
0xc7b30c: 0200 5c0c 0200 0200 0500 0800 7605 0000
          [0x0002, 0x0c5c(3164), 0x0002, 0x0002, 0x0005, 0x0008, ...]
```

Isso é o equivalente ao **S1 (stream-0 active height)** do A7 IV
(`0xd95c42`: `[0x0002, 0x0f66(3942), 0x0002]`).

**Importante:** `0xc7b30c` está **dentro** do Mode 3 record
(`0xc7abb0 + 0x75c`). Portanto os 198 patches do Antigravity **já incluíram
o S1** — mas ainda faltam os escalares S0/S3/S4, que ficam **fora** do record.

O campo `0x0005` em `0xc7b316` é o "sensor mode" (o A7 IV muda `06`→`05` no S0).

---

## 4. Semântica completa dos S0–S10 (do `ogsq_v3.json` da A7 IV)

| Patch | Addr A7 IV | O que muda | Old → New |
|---|---|---|---|
| **S4** | `0xc3dc92` | geometry row V offset | `6e01` → `0000` (366→0) |
| **S2** | `0xc98476` | crop V offset | `6e01` → `0000` |
| S5 | `0xd95c3e` | window field 11 | `0200` → `0800` |
| **S1** | `0xd95c42` | stream-0 active height | `660f` → `4012` (3942→4672) |
| S6 | `0xd95c46` | window field 15 | `0200` → `0800` |
| S10 | `0xd960b6` | aux/PDAF stream-1 rows | `5100` → `6100` |
| S7 | `0xd96428` | LC table V offset | `6e01` → `0000` |
| S8 | `0xd9642c` | LC field 6 (DrvParam+0x156) | `9301` → `1300` |
| S9 | `0xd965f4` | field 4 (sensor reg 0x19) | `3300` → `3b00` |
| **S3** | `0xd965fc` | readout length (line periods) | `db07` → `4e09` |
| **S0** | `0xfb4044` | register-matrix column (sensor mode 16→2) | `06000000` → `05000000` |

---

## 5. O que ainda falta (para o Antigravity caçar)

1. **S0 (register-matrix column) — região `0xfbxxxx`**
   - No A7 IV: `0xfb4044`, muda `06`→`05`.
   - Na FX30: preciso achar o equivalente na região `0xfbxxxx`
     (o `CP 0xfb4044` da FX30 é dados de ponteiros, não o seletor).
2. **S4 (geometry row V offset) — região `0xc3xxxx`**
   - No A7 IV: `0xc3dc92`, muda o V offset `366`→`0`.
   - Na FX30: essa região tem estrutura **`[0x0060, valor]` u32**
     (`0xc3dc00`: `0x00600d40`, `0x00600d41`, …), diferente dos pares
     `[reg, val]` u16 do A7 IV. Precisa decodificar.
3. **S3 (readout length) — provável `0xd9xxxx`**
   - No A7 IV: `0xd965fc`, `0x07db`(2011) → `0x094e`(2382).

---

## 6. Próximo passo recomendado

1. **Aplicar os 3 escalares juntos com o record** (S0 + S3 + S4 + os 198 campos):
   sem o S0 o sensor físico não muda a altura de leitura.
2. **Testar S0 primeiro**: achar o "sensor mode selector" na região `0xfbxxxx`
   da FX30 (o dump de 33 MB tem essa região).
3. **Se a tela acender**, ajustar encoder (av-cam.bin / libmpr.so) e LCD
   (`fx30lcd_v2`, `CP 0x1e2d3c`) pra fechar o 3:2.

---

## 7. Artefatos

- `mode3_to_mode10_diff.txt` — diff L (198 linhas `idx/mode3/mode10`)
- `fx30_cp_dump.bin` — dump CP 33 MB (0xC0000–0x2100000)
- `fx30_found_offsets.json` — outsz/dmmwb
- `outsz_write.c` / `outsz_lib.so` — helper outsz (verificado)
- `cp_dump.c` / `cp_dump_lib.so` — dumper (funciona)

---

## 8. Dica de segurança

Tudo em RAM. Bateria fora = stock. O ÚNICO risco é congelamento (recupera com
bateria). **Nunca** escrever em flash.

---

## 9. Resultados da caçada S0/S3/S4 (última rodada de análise)

### S0 (mode→column table) — candidatos na FX30

A faixa `0xfbxxxx` da FX30 **não** tem a tabela do S0: ali há **tabela de
ponteiros** (`0x7f...`) e **código ARM** (`0xe59ff068` = `ldr pc,[pc,#-0x68]`).
Em `0xea6000` há apenas strings (`[O] <CAM> us_kneegain`).

A FX30 tem **sequências de índice de modo** (mesmo formato do S0 do A7 IV,
u32 crescentes) em:

| CP | Sequência |
|---|---|
| `0xc00c68` | `02 03 04 05 06 07 00 01` (depois `02 03 11 12`) |
| `0xc30c78` | `02 03 04 05 06 07 00 01` |
| `0xc30c98` | `02 03 04 05 06 07 07 ...` |
| `0xc43b34` | `02 03 04 05 06 07 08 09 0a 0b 0c 0d` |

A sequência do A7 IV (`0xfb4030`) era `02 03 04 05 06 06 07 08 08 09 09 09 09 09 0a 0b`
(duplica o `06`; o S0 muda um deles `6→5`). A FX30 **não** tem essa duplicata —
o IMX684 usa outro esquema de seleção de coluna.

### S4 (geometry V offset) — não existe o padrão do A7 IV

O u32 do A7 IV em `0xc3dc92` é `0x016e0302` (valor `0x016e`=366, reg `0x0302`).
**Não há nenhuma ocorrência desse u32 no `cpapp-b.bin` da FX30.**

A região equivalente da FX30 (`0xc3dc00`) usa estrutura **`[0x0060, valor]` u32**:

```
0xc3dc00: 0x00600d40 0x00600d41 0x00600d42 0x00600d43 ...
0xc3dc80: 0x00600f55 0x00600f56 0x00600d54 ...
```

(high u16 = `0x0060` constante; low u16 incrementa — provavelmente uma LUT de
linhas/ganho, não a lista `[reg,val]` do A7 IV)

### S3 (readout length / line periods) — ainda não localizado

No A7 IV: `0xd965fc` `0x07db`(2011) → `0x094e`(2382).

### Conclusão para o Antigravity

O IMX684 **não** é um clone do IMX710. Os escalares S0/S4 não têm os mesmos
endereços nem a mesma estrutura. O caminho que resta é **seguir o ponteiro do
DrvParam (`0xc7b30c`) no dump** para achar o código que o lê, e nesse código
achar o índice/coluna do sensor e o registrador de V offset.

Sugestão de teste rápido (barato): aplicar **cada** uma das 3 tabelas de
índice acima como "S0" (mudando o índice do modo 3) **junto** com o patch dos
198 campos já validado, e ver se a imagem acende. Se nenhuma funcionar, partir
para o RE do código que lê o DrvParam.
