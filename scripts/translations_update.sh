#!/bin/bash

ARGUMENTS=""
if [ "$1" == "-no-obsolete" ]; then
ARGUMENTS="-no-obsolete"
fi
## circular inclusions workaround - we comment that buggy line
sed -i 's/include(3rdparty/##temp/g' ../src/lib/lib.pro

#lupdate $ARGUMENTS ../src/lib/lib.pro
lupdate $ARGUMENTS ../src/lib/lib.pro -ts ../translations/empty.ts

## uncomment it now
sed -i 's/##temp/include(3rdparty/g' ../src/lib/lib.pro

exit 0
