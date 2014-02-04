#!/bin/bash
# run_tests.sh

QMAKE="qmake"

if [ -n "$1" ]; then
 QMAKE=$1
fi

cd ../tests/autotests
($QMAKE DEFINES+=NO_SYSTEM_DATAPATH && make) || exit 1
cd ../../bin

./autotests
exit $?
