#!/bin/bash
#
# Usage: ./macdeploy.sh [<full-path-to-macdeployqt>]
#
# macdeployqt is usually located in QTDIR/bin/macdeployqt
# If path to macdeployqt is not specified, using it from PATH

MACDEPLOYQT="macdeployqt"
LIBRARY_NAME="libQupZilla.2.dylib"
PLUGINS="QupZilla.app/Contents/Resources/plugins"
QTPLUGINS="QupZilla.app/Contents/PlugIns"

if [ -n "$1" ]; then
 MACDEPLOYQT=$1
fi

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

if [ -z ${QTDIR+x} ]; then
  printf '\nPlease set the environment variable for the Qt platform folder.\n\texample:\n\t$ export QTDIR="$HOME/Qt/5.8/clang_64"\n'
  exit 1
else
  printf '\nCopying known, missing, Qt native library plugins to target bundle...\n'

  mkdir -p $QTPLUGINS

  FILE="$QTDIR/plugins/iconengines/libqsvgicon.dylib"
  if [ -f "$FILE" ]; then
    cp $FILE $QTPLUGINS/
  else
    echo "$FILE: No such file"
    exit 1
  fi

fi

# run macdeployqt
$MACDEPLOYQT QupZilla.app

# create final dmg image
cd ../mac
./create_dmg.sh
