#!/bin/bash

ARGUMENTS=""
if [ "$1" == "-no-obsolete" ]; then
ARGUMENTS="-no-obsolete"
fi
## circular inclusions workaround - we comment that buggy line
sed -i 's/include(3rdparty/##temp/g' ../src/src.pro

lupdate $ARGUMENTS ../src/main/main.pro
lupdate $ARGUMENTS ../src/main/main.pro -ts ../translations/empty.ts

## uncomment it now
sed -i 's/##temp/include(3rdparty/g' ../src/src.pro

read -p "Press [ENTER] to close terminal"
exit
