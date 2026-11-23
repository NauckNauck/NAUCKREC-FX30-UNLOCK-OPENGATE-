#!/bin/bash
# Limpeza de /tmp/jiritsu/libJit.so via sudo
cd "$(dirname "$0")" || exit 1
PY="/Users/nauck/Documents/sony_research/a7iv-open-gate-test/macos/.venv/bin/python"
CLEANER="$PWD/clean_service_partition.py"
PRIVATE="/Users/nauck/Documents/sony_research/a7iv-open-gate-test/windows/camera_private"
close() { echo; read -r -p "Pressione Enter para fechar... "; exit "$1"; }

sudo /bin/bash -c 'trap : INT; private=$1; py=$2; cleaner=$3
  "$py" -I -B "$cleaner"; status=$?
  [ -d "$private" ] && chown -R "$SUDO_UID:$SUDO_GID" "$private"; exit $status' run "$PRIVATE" "$PY" "$CLEANER"
close $?
