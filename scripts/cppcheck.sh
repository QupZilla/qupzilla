#!/bin/bash
#
# cppcheck
#

function check_code {
    cppcheck \
    -j 4 \
    --enable=style,performance,portability,missingInclude \
    --inline-suppr \
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

exit 0
