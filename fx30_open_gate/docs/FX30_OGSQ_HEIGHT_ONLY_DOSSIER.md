# FX30 Open Gate — Dossiê de Auditoria: Height-Only no Modo 3 (Largura 6K Preservada)

> Proposta do DeepSeek implementada pelo Antigravity:
> **Preservar 100% da largura horizontal 6K original do Modo 3 e esticar cirurgicamente apenas a altura e os offsets.**

---

## 1. Campos de Largura Mantidos Intactos (Original Stock Mode 3)
- **Left:**
  - `w2` (Raw width): `3192` (inalterado)
  - `w6` (Active width): `3176` (inalterado)
  - `w10` (Effective width): `3164` (inalterado)
- **Right:**
  - `w0` (X-offset): `2536` (inalterado)
  - `w2` (Raw width): `3192` (inalterado)
  - `w6` (Active width): `3116` (inalterado)
  - `w10` (Effective width): `3104` (inalterado)
- **Clocks, Timings e Registradores de Linha do Modo 3:** 100% preservados!

---

## 2. Campos de Altura Esticados Cirurgicamente (Left e Right)
- **`w3` (Raw height):** `3190 -> 4200`
- **`w7` (Active height):** `3164 -> 4168`
- **`w11` (Scaler height):** `3156 -> 4128`
- **`w943` (DrvParam height):** `3164 -> 4168`
- **`w1199` (Crop V offset):** `502 -> 0`

---

## 3. Registradores Globais de Geometria e Buffers
- **`S4` (Geometry V offset):** `CP 0xc365d4` -> `00 00` (era `f6 01` = 502)
- **`S2` (Crop V offset):** `CP 0xc9b308` -> `00 00` (era `f6 01` = 502)
- **`outsz` (`CP 0x155d444`):** `3840x2560`
- **`dmmwb` (`CP 0x2080858`):** `29.29 MB`

---

## 4. Estado do Binário
- **Arquivo:** `/Users/nauck/Documents/sony_research/fx30_open_gate/fx30_open_gate_lib.so` (24 KB)
- **Compilação:** Concluída com sucesso (`zig cc -target aarch64-linux-gnu`).
