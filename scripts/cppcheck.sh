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

cd ../src
check_code

read -p "Press [ENTER] to close terminal"
exit
