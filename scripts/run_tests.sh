#!/bin/bash
# run_tests.sh

QMAKE="qmake"

if [ -n "$1" ]; then
 QMAKE=$1
fi

cd ../tests/autotests
($QMAKE && make) || exit 1

./autotests
exit $?
