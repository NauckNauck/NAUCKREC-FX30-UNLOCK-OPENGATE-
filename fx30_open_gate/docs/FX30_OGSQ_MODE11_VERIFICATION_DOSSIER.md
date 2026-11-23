# FX30 Open Gate — Dossiê de Auditoria Final (Modo 11 + Right w0=2536)

> Dossiê atualizado após a observação do DeepSeek sobre o X-offset do canal Right.
> Binário gerado: `/Users/nauck/Documents/sony_research/fx30_open_gate/fx30_open_gate_lib.so` (30.016 bytes).

---

## 1. Auditoria dos Campos do Modo 11 Injetados no Modo 3 (`0xc7abb0..0xc7c320`)

O bloco de 6.000 bytes do Modo 11 agora possui o alinhamento estéreo perfeito entre Left e Right:

| Campo | Canal Left (`+0x000` / `0xc7abb0`) | Canal Right (`+0xbb8` / `0xc7b768`) | Estado / Valor Injetado | Função |
|---|---|---|---|---|
| **`w0` (X-offset)** | **0** | **2536** (`0x09e8`) | ✅ **Restaurado** (evita duplicação horizontal) | Offset horizontal do half-sensor |
| **`w3` (Raw height)** | **4200** | **4200** | ✅ Sincronizado | Linhas brutas lidas do sensor |
| **`w7` (Active height)**| **4168** | **4168** | ✅ Sincronizado | Altura ativa do frame |
| **`w11` (Eff/Scaler)** | **4128** | **4128** | ✅ Sincronizado | Altura de saída dos scalers |
| **`w943` (DrvParam H)**| **4168** | **4168** | ✅ Ajustado (era 686 L / 690 R) | Linhas ativas no DrvParam de vídeo |
| **`w946` (Stream Mode)**| **5** | **5** | ✅ Preservado nativo do Modo 11 | Modo contínuo de streaming de vídeo |
| **`w947` (Stream Val)** | **8** | **8** | ✅ Preservado nativo do Modo 11 | Parâmetro de clock de streaming de vídeo |
| **`w1199` (Crop V Off)**| **0** | **0** | ✅ Nativo 0 no Modo 11 | Crop vertical (zero = sem crop) |

---

## 2. Demais Injeções Concomitantes em RAM

1. **`outsz` (`0x155d444`):**
   - Injeta `00 0f 00 0a` (3840 x 2560 3:2).
2. **`dmmwb` (`0x2080858..0x2080970`):**
   - Injeta `00 00 bf 01` (buffer expandido para 29.29 MB / `0x01bf0000`).
3. **`S4` Geometry Row V offset (`CP 0xc365d4`):**
   - Injeta `00 00` (zera o offset de 502 linhas da geometria global).
4. **`S2` Crop Row V offset (`CP 0xc9b308`):**
   - Injeta `00 00` (zera o offset de 502 linhas do crop global).

---

## 3. Verificação Hexadecimal dos Primeiros Bytes dos Dois Canais

- **Left (início em `0xc7abb0` / offset 0 do buffer):**
  - `w0` = `00 00` (0)
  - `w1` = `00 00` (0)
  - `w2` = `3c 08` (2108)
  - `w3` = `68 10` (4200)
  - `w7` = `48 10` (4168)
  - `w11` = `20 10` (4128)

- **Right (início em `0xc7b768` / offset 3000 do buffer):**
  - `w0` = `e8 09` (**2536** ✅)
  - `w1` = `00 00` (0)
  - `w3` = `68 10` (4200 ✅)
  - `w7` = `48 10` (4168 ✅)
  - `w11` = `20 10` (4128 ✅)
  - `w943` (offset 4886) = `48 10` (4168 ✅)
