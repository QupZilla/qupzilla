#!/bin/bash
#
# Usage: ./macdeploy.sh <full-path-to-macdeployqt>
#
# macdeployqt is usually located in QTDIR/bin/macdeployqt

if [ -z "$1" ]; then
 echo "Required parameter missing for full path to macdeployqt"
 exit 1
fi

MACDEPLOYQT=$1
QTDIR="`dirname $MACDEPLOYQT`/.."
LIBRARY_NAME="libQupZilla.2.dylib"
PLUGINS="QupZilla.app/Contents/Resources/plugins"

# cd to directory with bundle
test -d bin || cd ..
cd bin

# copy libQupZilla into bundle
cp $LIBRARY_NAME QupZilla.app/Contents/MacOS/

# copy all QupZilla plugins into bundle
test -d $PLUGINS || mkdir $PLUGINS
cp plugins/*.dylib $PLUGINS/

# fix libQupZilla
install_name_tool -change $LIBRARY_NAME @executable_path/$LIBRARY_NAME QupZilla.app/Contents/MacOS/QupZilla

# fix plugins
for plugin in $PLUGINS/*.dylib
do
 install_name_tool -change $LIBRARY_NAME @executable_path/$LIBRARY_NAME $plugin
done

# run macdeployqt
$MACDEPLOYQT QupZilla.app -qmldir=$PWD/../src/lib/data/data

# create final dmg image
cd ../mac
./create_dmg.sh
