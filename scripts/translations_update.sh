#!/bin/bash

ARGUMENTS=""
if [ "$1" == "-no-obsolete" ]; then
ARGUMENTS="-no-obsolete"
fi

# circular inclusions workaround - we comment that buggy line
sed -i 's/include(3rdparty/##temp/g' ../src/lib/lib.pro

cd ../src/lib/
rm ../../translations/empty.ts
lupdate $ARGUMENTS lib.pro -ts ../../translations/empty.ts
cd -

# uncomment it now
sed -i 's/##temp/include(3rdparty/g' ../src/lib/lib.pro

# it is needed to double <numerusform></numerusform> for transifex
awk '{ print $0; \
       if (index($0, "<numerusform></numerusform>")) \
           print $0; \
     }' ../translations/empty.ts > tmp.ts
mv tmp.ts ../translations/empty.ts

exit 0
