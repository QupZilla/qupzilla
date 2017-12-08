#!/bin/bash

TMP=/tmp
BUNDLE_PATH=bin
test -d bin || BUNDLE_PATH=../bin

WORK_TEMPLATE=tmp-QupZilla-release.dmg.sparseimage

# NOTE: Value must currently match -volname exactly or an error happens in AppleScript with Finder
VOLUME_TEMPLATE=QupZilla

echo "Ensuring working disk image template is not in use…"
hdiutil detach "$TMP/$WORK_TEMPLATE"

echo "Creating writable working disk image template…"
hdiutil create -size 200m "$TMP/$WORK_TEMPLATE" -type SPARSE -fs HFS+ -volname "QupZilla"

  echo "Attaching working disk image template for modification…"
  hdiutil attach "$TMP/$WORK_TEMPLATE" -mountpoint "$TMP/$VOLUME_TEMPLATE"

    echo "Creating background image folder…"
    mkdir "$TMP/$VOLUME_TEMPLATE/.background"

    echo "Copying background image…"
    cp images/qupzilla-dmg-background.png "$TMP/$VOLUME_TEMPLATE/.background/"

    echo "Creating volume icon set…"
    ICON=qupzilla-dmg-icon
    ICONEXT=png
    ICONSETDIR=images/$ICON.iconset

    mkdir -p "$ICONSETDIR"

    # Convert last with qlmanage since sips does not do SVG
    qlmanage -t -s 1024 -o "$ICONSETDIR" "images/$ICON.$ICONEXT"
    mv "$ICONSETDIR/$ICON.$ICONEXT.png" "$ICONSETDIR/icon_512x512@2x.png"

    # Convert remaining with sips since qlmanage does not do 16 pixels
    sizes=( 512 256 128 64 32 16 )
    for size in ${sizes[@]}
    do
      sips -Z $size -s format png "$ICONSETDIR/icon_512x512@2x.png" --out "$ICONSETDIR/icon_${size}x${size}.png"

      halfsize=$((size / 2))
      if [[ size -ne 16 ]]; then
        ln "$ICONSETDIR/icon_${size}x${size}.png" "$ICONSETDIR/icon_${halfsize}x${halfsize}@2x.png"
      fi
    done

    echo "Creating application reference folder…"
    mkdir "$TMP/$VOLUME_TEMPLATE/QupZilla.app"

    echo "Creating symbolic link to global Applications folder…"
    ln -s /Applications "$TMP/$VOLUME_TEMPLATE/Applications"

    echo "Setting some proprietary window modifications here with AppleScript…"
    osascript create_dmg.scpt

    sleep 5

    echo "Registering that a custom icon is being set…"
    # iconutil will only pick what it needs
    iconutil -c icns "$ICONSETDIR" -o "$TMP/$VOLUME_TEMPLATE/.VolumeIcon.icns"
    SetFile -a C "$TMP/$VOLUME_TEMPLATE"

    echo "Copying application bundle contents…"
    cp -fpR "$BUNDLE_PATH/QupZilla.app/Contents" "$TMP/$VOLUME_TEMPLATE/QupZilla.app"

    echo "Blessing folder to automatically open on mount…"
    bless --folder "$TMP/$VOLUME_TEMPLATE" --openfolder "$TMP/$VOLUME_TEMPLATE"

  echo "Detaching working disk image template from write…"
  hdiutil detach "$TMP/$VOLUME_TEMPLATE"

echo "Compacting working disk image…"
hdiutil compact "$TMP/$WORK_TEMPLATE"

echo "Converting working disk image to read only…"
rm "$BUNDLE_PATH/QupZilla.dmg"
hdiutil convert "$TMP/$WORK_TEMPLATE" -format UDBZ -o "$BUNDLE_PATH/QupZilla.dmg"

echo  "Cleaning up"
rm -Rf "$ICONSETDIR"
rm "$TMP/$WORK_TEMPLATE"
