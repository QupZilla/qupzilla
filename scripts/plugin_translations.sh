#!/bin/bash

cd ../src/plugins

for pluginPro in */*.pro
do
 #lupdate $pluginPro -no-obsolete
 lupdate $pluginPro -no-obsolete -ts $pluginPro/../translations/empty.ts
done

exit 0
