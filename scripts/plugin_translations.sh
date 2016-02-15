#!/bin/bash

cd ../src/plugins

for pluginPro in */*.pro
do
    # circular inclusions workaround - we comment that buggy line
    sed -i 's/include(../##temp/g' $pluginPro

    lupdate $pluginPro -no-obsolete -ts $pluginPro/../translations/empty.ts

    # uncomment it now
    sed -i 's/##temp/include(../g' $pluginPro
done

exit 0
