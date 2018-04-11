#!/usr/bin/env bash

RTU_SLAVE_PID=/tmp/rtu_slave.pid

CURRENT_DIR="$(dirname "$(realpath "$0")")"
. $CURRENT_DIR/subprocess_helper.sh

check_preconditions() {
    python -c "import pymodbus" || exit 1
}

# check argument count
## https://stackoverflow.com/questions/4341630/checking-for-the-correct-number-of-arguments
if [ "$#" -ne 1 ]; then
  echo "[E] usage: $0 <start|stop>" >&2
  exit 1
fi

check_preconditions
case "$1" in
    up|start)
        CMD="python ${CURRENT_DIR}/rtu_slave.py &"
        run_cmd_save_pid "$CMD" $RTU_SLAVE_PID
        ;;
    down|stop)
        kill_pid $RTU_SLAVE_PID
        ;;
esac
