#!/usr/bin/env bash

MBUSD_BIN=${MBUSD_BIN:=mbusd}
MBUSD_ARGS="-d -L - -v9 -p /tmp/pts0 -s 19200 -P 1025"
MBUSD_PID=/tmp/mbusd.pid

# check argument count
## https://stackoverflow.com/questions/4341630/checking-for-the-correct-number-of-arguments
if [ "$#" -ne 1 ]; then
  echo "[E] usage: $0 <start|stop>" >&2
  exit 1
fi

check_preconditions() {
    # check if mbusd location is set
    if ! [ -x "$MBUSD_BIN" ]; then
      echo "[E] executable binary not found: ${MBUSD_BIN}" >&2
      exit 1
    fi
}

check_preconditions
CURRENT_DIR="$(dirname "$(realpath "$0")")"
. $CURRENT_DIR/subprocess_helper.sh

case "$1" in
    up|start)
        CMD="$MBUSD_BIN $MBUSD_ARGS &"
        run_cmd_save_pid "$CMD" $MBUSD_PID
        ;;
    down|stop)
        kill_pid $MBUSD_PID
        ;;
esac

