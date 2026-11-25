#!/bin/bash
cd "$(dirname "$0")" || exit 1
PY="/Users/nauck/Documents/sony_research/a7iv-open-gate-test/macos/.venv/bin/python"
echo "================================================================"
echo "  SONY FX30 (FW 6.01) — V60 CARD UNLOCK + OPEN GATE 3:2"
echo "================================================================"
echo "1) Step 1 Isolado (V60 Unlock + fx30_ogsq_v6 Squeezed)"
echo "2) Full Open Gate 3:2 3840x2560 (V60 + fx30og_v3 + v6 + outsz/LCD)"
echo "3) Cura USB em Movie + Read-Probe (--probe)"
echo "4) Apenas V60 Card Unlock (--card-only)"
read -r -p "Escolha [1/2/3/4]: " choice
case "$choice" in
  1) FLAG="--step1" ;;
  2) FLAG="--full" ;;
  3) FLAG="--probe" ;;
  4) FLAG="--card-only" ;;
  *) echo "Opcao invalida."; exit 1 ;;
esac
sudo "$PY" -I -B "$PWD/run_fx30_complete.py" "$FLAG"
read -r -p "Pressione Return para fechar..."
