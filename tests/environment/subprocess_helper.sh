#!/usr/bin/env bash

# @arg1 command to be executed in background as string
# @arg2 path to PID file
run_cmd_save_pid() {
    local _CMD="$1"
    local _PID_FILE=$2

    if [[ -e $_PID_FILE ]]; then
    	_PID=`cat $_PID_FILE`
    	if ps -p $_PID > /dev/null; then
        	echo "[D] PID file ($_PID_FILE) exists, done"
        	return 1
    	else
			echo "[D] removing stale PID file ($_PID_FILE)"
			rm -f $_PID_FILE
		fi
    fi

    echo "[I] running $_CMD in background..."
    eval $_CMD
    if [[ $? -ne 0 ]] ; then
      echo "[E] $_CMD call produced errors"
      return 2
    fi
    ## https://stackoverflow.com/questions/9890062/how-to-run-a-program-and-know-its-pid-in-linux
    echo $! > $_PID_FILE
}
# @arg1 path to PID file
kill_pid() {
  local _PID_FILE=$1

  if [[ ! -e $_PID_FILE ]]; then
     return 0
  fi

  echo "[I] killing pid file: $_PID_FILE"
  kill `cat $_PID_FILE`
  rm -f $_PID_FILE
}