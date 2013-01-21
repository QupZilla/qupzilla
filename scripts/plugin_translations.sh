#!/bin/bash

cd ../src/plugins

for pluginPro in */*.pro
do
 lupdate $pluginPro -no-obsolete
done

exit 0
