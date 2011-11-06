#!/bin/bash
#
# cppcheck
#

echo "cppcheck..."

cd ../src
cppcheck \
--enable=all \
--force \
--verbose \
. > /dev/null

read -p "Press [ENTER] to close terminal"
exit
