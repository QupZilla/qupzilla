#!/bin/bash

cd ../src/plugins

for pluginPro in */*.pro
do
 lupdate $pluginPro -no-obsolete
done

read -p "Press [ENTER] to close terminal"
exit
