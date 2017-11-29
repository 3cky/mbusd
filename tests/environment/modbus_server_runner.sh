#!/usr/bin/env bash

MBUS_SERVER_PID=/tmp/modbus_server.pid

. ./subprocess_helper.sh

check_preconditions() {
    #TODO check if python module 'pymodbus' is installed
    #python -c "import foo"
    true
}

# check argument count
## https://stackoverflow.com/questions/4341630/checking-for-the-correct-number-of-arguments
if [ "$#" -ne 1 ]; then
  echo "Usage: $0 up|down" >&2
  exit 1
fi

check_preconditions
case "$1" in
    up|start)
        #TOOO obtain current directory
        CURRENT_DIR="$(dirname "$(realpath "$0")")"
        CMD="python ${CURRENT_DIR}/modbus_server_mock.py &"
        run_cmd_save_pid "$CMD" $MBUS_SERVER_PID
        ;;
    down|stop)
        kill_pid $MBUS_SERVER_PID
        ;;
esac

