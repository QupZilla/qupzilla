#!/bin/bash
#
# cppcheck
#

function check_code {
    cppcheck \
    --enable=all \
    --force \
    --verbose \
    . > /dev/null
}

echo "cppcheck..."

cd ../src/plugins
check_code

cd ../lib/
check_code

cd ../main
check_code

read -p "Press [ENTER] to close terminal"
exit
