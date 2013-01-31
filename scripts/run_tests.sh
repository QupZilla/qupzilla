#!/bin/bash
# run_tests.sh

cd ../tests/autotests
qmake DEFINES+=NO_SYSTEM_DATAPATH && make
cd ../../bin

./autotests
exit $?
