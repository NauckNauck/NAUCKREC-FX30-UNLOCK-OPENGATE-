#!/bin/bash
# FX30 OPEN GATE 3:2 — FASE 3 DEFINITIVA
# Passo 0: Limpeza de resíduos
# Passo 1: Encoder AP (fx30og_v2) -> 3840x2560 em av-cam.bin e libmpr.so
# Passo 2: Helper Unificado Fase 3 (S3=2020 + 180 campos + S1=3968 + outsz 2560 + DMM + LCD)
cd "$(dirname "$0")" || exit 1

PY="/Users/nauck/Documents/sony_research/a7iv-open-gate-test/macos/.venv/bin/python"
APPLY="/Users/nauck/Documents/sony_research/a7iv-open-gate-test/windows/tools/camera_tests/CT6_memory_read_check/fx30_apply.py"
CP_RUNNER="$PWD/send_fx30_phase3.py"
CLEANER="$PWD/clean_service_partition.py"
PRIVATE="/Users/nauck/Documents/sony_research/a7iv-open-gate-test/windows/camera_private"

close() { echo; read -r -p "Pressione Enter para fechar... "; exit "$1"; }

if [ ! -x "$PY" ]; then
  echo "Ambiente Python nao encontrado: $PY"
  close 1
fi

echo "======================================================================"
echo "🎯 SONY FX30 — FASE 3: OPEN GATE 3:2 TOTAL (3840x2560)"
echo "100% Volátil em RAM. Zero escrita permanente."
echo "======================================================================"
echo

# Força desmontagem de volumes de cartões se estiverem montados
diskutil unmountDisk force /dev/disk5 2>/dev/null || true
diskutil unmountDisk force /dev/disk6 2>/dev/null || true

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
  echo "✅ PASSO 1 (ENCODER) APLICADO COM SUCESSO NA RAM!"
  echo "ATENCAO: NAO DESLIGUE A CAMERA! (Nao retire a bateria)"
  echo "Apenas desconecte e reconecte o cabo USB-C (ou aperte o botao central)"
  echo "para a camera voltar a mostrar Mass Storage no Mac."
  echo "======================================================================"
  read -r -p "Quando a camera reconectar em Mass Storage, pressione Enter para rodar o Passo 2: "

  echo
  echo ">>> [Passo 2/2] Injetando Helper Fase 3 (S3 Line Periods + 180 Campos + Scaler)..."
  "$py" -I -B "$cp_runner"
  status=$?

  [ -d "$private" ] && chown -R "$SUDO_UID:$SUDO_GID" "$private"
  exit $status' run "$PRIVATE" "$PY" "$APPLY" "$CP_RUNNER" "$CLEANER"

close $?
