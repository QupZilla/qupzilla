#!/bin/bash
#
# Usage: ./macdeploy.sh [<full-path-to-macdeployqt>]
#
# macdeployqt is usually located in QTDIR/bin/macdeployqt
# If path to macdeployqt is not specified, using it from PATH

MACDEPLOYQT="macdeployqt"
LIBRARY_NAME="libQupZilla.2.dylib"
PLUGINS="QupZilla.app/Contents/Resources/plugins"

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

# prompt and optionally copy additional Qt native plugin(s) into bundle
echo -n "Do you wish to redistribute known, missing, Qt plugins (y/n)? "
old_stty_cfg=$(stty -g)
stty raw -echo
answer=$( while ! head -c 1 | grep -i '[yn]'; do true; done )
stty $old_stty_cfg
if echo "$answer" | grep -iq "^y"; then
  printf '\nCopying known, missing, Qt native plugins to target...\n'

  cp $QTDIR/plugins/iconengines/libqsvgicon.dylib $PLUGINS
else
  printf '\nChecking for prior deploy image libraries at target...\n'

  rm $PLUGINS/libqsvgicon.dylib
fi

# run macdeployqt
$MACDEPLOYQT QupZilla.app

# create final dmg image
cd ../mac
./create_dmg.sh
