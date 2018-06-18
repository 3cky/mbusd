#!/usr/bin/env bash

CWD="$(dirname "$(realpath "$0")")"

if ! [ -x "$1" ]; then
  echo "[E] usage: $0 <mbusd_binary_path>" >&2
  exit 1
fi

export MBUSD_BIN=$1
$CWD/run_itests.py "$MBUSD_BIN" || exit 1

exit 0
