#!/usr/bin/env bash

SOCAT_BIN=socat
SOCAT_ARGS="-d -d pty,raw,echo=0,link=/tmp/pts0 pty,raw,echo=0,link=/tmp/pts1"
SOCAT_PID_FILE=/tmp/socat.pid

CURRENT_DIR="$(dirname "$(realpath "$0")")"
. $CURRENT_DIR/subprocess_helper.sh

check_preconditions() {
    # check if socat is installed
    ## https://stackoverflow.com/questions/592620/check-if-a-program-exists-from-a-bash-script
    if ! [ -x "$(command -v $SOCAT_BIN)" ]; then
      echo '[E] please install $SOCAT_BIN ' >&2
      exit 1
    fi
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
        SOCAT_CMD="$(command -v socat) ${SOCAT_ARGS} &"
        run_cmd_save_pid "$SOCAT_CMD" $SOCAT_PID_FILE
        ;;
    down|stop)
        kill_pid $SOCAT_PID_FILE
        ;;
esac

