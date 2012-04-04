#!/bin/bash
COMMAND=$1

if [ $COMMAND = "" ]; then
 $COMMAND="macdeployqt"
fi

# cd to directory with bundle
cd ../build

# copy libQupZilla into bundle
cp libQupZilla* QupZilla.app/Contents/MacOS/QupZilla

# copy all plugins into bundle
test -d QupZilla.app/Contents/Resources/plugins || mkdir QupZilla.app/Contents/Resources/plugins
cp plugins/*.dylib QupZilla.app/Contents/Resources/plugins

# fix libQupZilla
install_name_tool -change libQupZilla.1.dylib @executable_path/libQupZilla.1.dylib QupZilla.app/Contents/MacOS/QupZilla

# fix plugins
for plugin in QupZilla.app/Contents/Resources/plugins*.dylib
do
 install_name_tool -change libQupZilla.1.dylib @executable_path/libQupZilla.1.dylib $plugin
done

# run macdeployqt
$COMMAND QupZilla.app
