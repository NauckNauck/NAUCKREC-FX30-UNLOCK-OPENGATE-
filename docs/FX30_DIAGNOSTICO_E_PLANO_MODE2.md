# Sony FX30 (FW 6.01 / IMX684) — Diagnóstico Técnico Consolidado & Plano Mode 2 (Coluna 1)

**Data:** 24 de Novembro de 2026  
**Câmera:** Sony ILME-FX30 (FW 6.01, BIONZ XR, CP ARM32 Thumb-2)  
**Sensor:** Sony IMX684 (APS-C 26 MP — $6192 \times 4128$ Still 3:2 / $5952 \times 3348$ Vídeo 4K 16:9)  

---

## 1. Por Que os Testes Anteriores Falharam (e Por Que o Sensor É Capaz)

### 1.1 Refutação do "Limite de Clock"
A hipótese de que o sensor da FX30 não tem folga de clock para rodar acima de 3348 linhas em 23.976 fps está incorreta:
- **Prova Empírica:** O Live View do modo Foto (Still) lê **$6192 \times 4128$** continuamente a 24/30 fps sem overclock.
- **Prova Matemática na Coluna 1 (10-bit Vídeo):**
  - `VMAX` do Timing Generator = `2167` half-lines.
  - Stock 4K 16:9 (`3348` linhas): `Active = 1680`, `VBLANK = 487` half-lines (**974 linhas livres**).
  - Alvo Open Gate 3:2 (`3968` linhas, $5952 \times 3968$): `Active = 1990`, `VBLANK = 177` half-lines (**354 linhas livres**), mantendo exatamente `HMAX = 340`, `VMAX = 4410` (`23.976 fps`).
  - **Conclusão:** Não é necessário alterar `T.tim` (`HMAX`/`VMAX`). Alterações manuais em `T.tim` foram a causa direta dos soft reboots.

### 1.2 Os 4 Erros Cruzados dos Testes Anteriores
1. **Mistura de Colunas e `n0 = 0x01` (Single-Shot):**
   - Alterar `S0` para Coluna 11 (`0x0b`) enquanto os patches de `VBLANK`/`active` estavam na Coluna 1 — ou sem manter `n0 = 0x0f` (streaming contínuo) e ajustar o link de **10-bit (Mode 2)** para **12-bit/14-bit (Still)** (`0x274`–`0x2a8`, conforme documentado em `ogsq_s3_v5.json`) — causa telas pretas, cores rosadas/amareladas ou watchdog reboot.
2. **Injeção em Modo Vídeo em vez de Modo Foto (Still → Movie):**
   - Nos kits validados do ArcaneLantern (`fx3-open-gate-test` / `a7siii-open-gate-test` / `a7iv-open-gate-test`), a injeção **sempre** ocorre com a câmera ligada em **Modo Foto (M/P/A/S)** após cold boot.
   - O disassembly de `cpapp-b.bin` confirma que `0x8e8f3e` (que resolve ambos os modos via `0x8e99e6` e chama `VTable[+0x100]` em `0x8e8218` para gravar a matriz no buffer P2S `0x01123xxx`) dispara na transição de modo **Foto → Vídeo (`MODE → Movie`)**.
3. **Mode 3 vs. Mode 2 e Omissão dos 180 Campos:**
   - O modo 4K 24p consome o **Mode 2 (`0xc79440` L / `0xc79ff8` R)**, não o Mode 3 (`0xc7abb0`).
   - Os 180 campos do Mode 2 escalados para `3968` linhas (`+620` ativas, `+626` bounding, `+310` half, `+155` quarter, `+0x784 = 6667`, `+0x95e = 0`) estão mapeados em `docs/fx30_mode2_180_fields_layout.json`.
4. **Endereços Falsos e Prescalers:**
   - `0xc9b308` e `0xc365d4` (falsos `S2`/`S4`) são IDs sequenciais, não offsets verticais — não devem ser zerados.
   - Nós `7` (`0x09`), `8` (`0x22`) e `10` (`0x29`) devem permanecer em stock na Coluna 1 para evitar deslocamento de fase Bayer (tom rosado).

---

## 2. O Que Aproveitar de `fx3-open-gate-test` (FX3 7.02)

- **Status do Repo da FX3:** Ainda não testado em FX3 real (*"Not run on an FX3 yet"*); é porta do sensor da A7S III (IMX510), não da FX30 (IMX684).
- **De-Squeeze Display (`lcd32dsq_fx3_v1`):** Em vez de macro no botão C3, a FX3 usa o próprio **De-Squeeze Display** nativo (`1.3x`, `1.5x`, `1.65x`, `2.0x` sem crop 4:3) ajustado para o quadro 3:2. Esse mesmo recurso existe no menu da FX30 e é o caminho ideal para o Passo 2.

---

## 3. Plano de Execução em 2 Passos

### Passo 1 — Validar Sensor + ISP Front-End Isolados ("Squeezed Open Gate")
**Arquivo:** `fx30_open_gate/sources/fx30_mode2_squeezed_og.c` (371 patches `u16` + 4 patches `u8` validados contra `cpapp-b.bin`).

- **Escopo:**
  - Mantém **Coluna 1** (`10-bit` nativo, `n0 = 0x0f`).
  - Altera apenas Nós `3–6` da Coluna 1 (`0xb1`, `0x00`, `0xc6`, `0x07`) — Nós `7, 8, 10` intocados.
  - Aplica os **180 campos do Mode 2** (`0xc79440` L / `0xc79ff8` R) para `3968` linhas.
  - Sincroniza `S1` no **Mode 2 Preview (`0x00e027f0`)** e **Mode 6 4K 24p (`0x00e02888`)** = `3968`, `S3 = 2020`, `S9 = 47`, `S7 = 0`.
  - **Não** altera `outsz`, `dmmwb`, `LCD`, `av-cam` nem `T.tim` (para isolar o sensor sem risco de estouro de DMM).
- **Ritual de Teste Obrigatório:**
  1. **Cold boot** (bateria fora por 10s).
  2. Ligar a câmera em **Modo Foto (M/P/A/S)** com `XAVC HS 4K 24p` pré-selecionado para Vídeo.
  3. Conectar em **Mass Storage** e injetar o helper compilado (`m2_ok = OK`).
  4. Desconectar o USB e **só então apertar `MODE → Movie`** (se entrar glitchado, iniciar e parar um REC rápido de 1s).
- **Critério de Sucesso:** Live view contínuo, cores normais (sem rosa) e imagem **verticalmente comprimida** dentro da janela 16:9 stock (`3840×2160`).

### Passo 2 — Saída Nativa `3840×2560` + LCD 3:2
Com o Passo 1 estável e gravando:
1. Ligar `outsz` (`0x155d446`: `0x0870 → 0x0a00`) + `dmmwb` (`0x2080860` / `0x2080874` → `0x01bf0000`, `29.29 MB`).
2. Aplicar `fx30og_v2` (`av-cam.bin` + `libmpr.so` em `3840×2560`).
3. Aplicar `fx30lcd_v2` (e portar os ratios do `De-Squeeze Display` inspirados em `lcd32dsq_fx3_v1`).
