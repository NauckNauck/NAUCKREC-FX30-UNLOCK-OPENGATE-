#!/bin/bash
# FX30 Open Gate 3:2 Runner for macOS
# 100% Volatile RAM injection. Battery pull clears everything.
cd "$(dirname "$0")" || exit 1

PY="/Users/nauck/Documents/sony_research/a7iv-open-gate-test/macos/.venv/bin/python"
RUNNER="$PWD/send_fx30_open_gate.py"
PRIVATE="/Users/nauck/Documents/sony_research/a7iv-open-gate-test/windows/camera_private"

close() { echo; read -r -p "Pressione Enter para fechar... "; exit "$1"; }

if [ ! -x "$PY" ]; then
  echo "Ambiente Python nao encontrado: $PY"
  close 1
fi

echo "======================================================================"
echo "SONY FX30 — INJECAO VOLATIL DE OPEN GATE 3:2 (3840x2560)"
echo "1. outsz: Resolução 3840x2560 em CP 0x155d444"
echo "2. dmmwb: Buffer DMM expandido para 29.29 MB (0x1bf0000)"
echo "3. ogsq: Geometria do Modo 3:2 Full Height Nativo (4168 linhas)"
echo "======================================================================"
echo

sudo /bin/bash -c 'trap : INT; private=$1; py=$2; runner=$3
  "$py" -I -B "$runner"; status=$?
  [ -d "$private" ] && chown -R "$SUDO_UID:$SUDO_GID" "$private"; exit $status' run "$PRIVATE" "$PY" "$RUNNER"

close $?
