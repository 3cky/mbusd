#!/usr/bin/env bash

CWD="$(dirname "$(realpath "$0")")"

if ! [ -x "$1" ]; then
  echo "[E] usage: $0 <mbusd_binary_path>" >&2
  exit 1
fi

export MBUSD_BIN=$1

function setup() {
    echo "[I] do test environment setup"
    $CWD/environment/socat_runner.sh start || return 1
    # $CWD/environment/rtu_slave_runner.sh start || return 1
    $CWD/environment/mbusd_runner.sh start || return 1
}

function teardown() {
    echo "[I] do test environment teardown"
    $CWD/environment/mbusd_runner.sh stop
    # $CWD/environment/rtu_slave_runner.sh stop
    $CWD/environment/socat_runner.sh stop
}
trap teardown EXIT

setup || (echo "failed to setup" && exit 1)

$CWD/run_itests.py || exit 1

exit 0
