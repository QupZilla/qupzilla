#!/bin/bash
#
# Usage: ./macdeploy.sh [<full-path-to-macdeployqt>]
#
# macdeployqt is usually located in QTDIR/bin/macdeployqt
# If path to macdeployqt is not specified, using it from PATH

MACDEPLOYQT="macdeployqt"
LIBRARY_NAME="libQupZilla.1.dylib"

if [ -n "$1" ]; then
 MACDEPLOYQT=$1
fi

# cd to directory with bundle
test -d bin || cd ..
cd bin

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
$MACDEPLOYQT QupZilla.app

# create final dmg image
cd ../mac
./create_dmg.sh

