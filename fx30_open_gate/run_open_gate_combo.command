#!/bin/bash
# FX30 Open Gate Completo e Perfeito:
# Passo 0: Limpa /tmp/jiritsu/libJit.so
# Passo 1: Aplica fx30og_v2 (Encoder 3840x2560 em av-cam.bin & libmpr.so)
# Pausa para o usuário dar ok na reconexão USB (sem desligar a câmera!)
# Passo 2: Aplica CP Helper (Height-Only Mode 3 + outsz)
cd "$(dirname "$0")" || exit 1

PY="/Users/nauck/Documents/sony_research/a7iv-open-gate-test/macos/.venv/bin/python"
APPLY="/Users/nauck/Documents/sony_research/a7iv-open-gate-test/windows/tools/camera_tests/CT6_memory_read_check/fx30_apply.py"
CP_RUNNER="$PWD/send_fx30_open_gate.py"
CLEANER="$PWD/clean_service_partition.py"
PRIVATE="/Users/nauck/Documents/sony_research/a7iv-open-gate-test/windows/camera_private"

close() { echo; read -r -p "Pressione Enter para fechar... "; exit "$1"; }

if [ ! -x "$PY" ]; then
  echo "Ambiente Python nao encontrado: $PY"
  close 1
fi

echo "======================================================================"
echo "SONY FX30 — INJECAO TOTAL OPEN GATE 3:2 (3840x2560)"
echo "1. Limpando resíduos em /tmp/jiritsu/libJit.so"
echo "2. Passo 1: Encoder (fx30og_v2)"
echo "3. Passo 2: Sensor & Scaler (CP Helper - Height-Only + outsz)"
echo "======================================================================"
echo

sudo /bin/bash -c 'trap : INT; private=$1; py=$2; apply=$3; cp_runner=$4; cleaner=$5
  echo ">>> [Passo 0] Limpando libJit.so residual..."
  "$py" -I -B "$cleaner" 2>/dev/null

  echo
  echo ">>> [Passo 1/2] Injetando fx30og_v2 (Encoder 3840x2560)..."
  "$py" -I -B "$apply" --open-gate-only --ntsc -y
  status=$?
  if [ $status -ne 0 ]; then
    echo "FALHA no Passo 1 (fx30og_v2). Abortando."
    exit $status
  fi

  echo
  echo "======================================================================"
  echo "PASSO 1 CONCLUIDO COM SUCESSO NA RAM!"
  echo "ATENCAO: NAO DESLIGUE A CAMERA! (Nao retire a bateria)"
  echo "Apenas desconecte e reconecte o cabo USB-C (ou aperte o botao central)"
  echo "para a camera voltar a mostrar Mass Storage no Mac."
  echo "======================================================================"
  read -r -p "Quando a camera reconectar em Mass Storage, pressione Enter para rodar o Passo 2: "

  echo
  echo ">>> [Passo 2/2] Injetando CP Helper (Height-Only Mode 3 + outsz)..."
  "$py" -I -B "$cp_runner"
  status=$?

  [ -d "$private" ] && chown -R "$SUDO_UID:$SUDO_GID" "$private"
  exit $status' run "$PRIVATE" "$PY" "$APPLY" "$CP_RUNNER" "$CLEANER"

close $?
