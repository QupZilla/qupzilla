#!/bin/bash
COMMAND=$1
LIBRARY_NAME="libQupZilla.1.dylib"

if [ $COMMAND = "" ]; then
 $COMMAND="macdeployqt"
fi

# cd to directory with bundle
test -d build || cd ..
cd build

# copy libQupZilla into bundle
cp $LIBRARY_NAME QupZilla.app/Contents/MacOS/

# copy all plugins into bundle
test -d QupZilla.app/Contents/Resources/plugins || mkdir QupZilla.app/Contents/Resources/plugins
cp plugins/*.dylib QupZilla.app/Contents/Resources/plugins/

# fix libQupZilla
install_name_tool -change $LIBRARY_NAME @executable_path/$LIBRARY_NAME QupZilla.app/Contents/MacOS/QupZilla

# fix plugins
for plugin in QupZilla.app/Contents/Resources/plugins/*.dylib
do
 install_name_tool -change $LIBRARY_NAME @executable_path/$LIBRARY_NAME $plugin
done

# run macdeployqt
$COMMAND QupZilla.app
