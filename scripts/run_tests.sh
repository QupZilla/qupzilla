#!/bin/bash
# run_tests.sh

cd ../tests/autotests
(qmake DEFINES+=NO_SYSTEM_DATAPATH && make) || exit 1
cd ../../bin

./autotests
exit $?
