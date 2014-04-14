#!/bin/bash

BUNDLE_PATH=bin
test -d bin || BUNDLE_PATH=../bin

echo "We just want to make sure it's not in use"
hdiutil detach /tmp/tmp-release-qupzilla

echo "Creating writable DMG from template"
hdiutil convert QupZilla-template.dmg -format UDRW -o /tmp/tmp-qupzilla.dmg

echo "Attach DMG for write"
hdiutil attach /tmp/tmp-qupzilla.dmg -mountpoint /tmp/tmp-qupzilla-release

echo "Copying…"
cp -fpR $BUNDLE_PATH/QupZilla.app/Contents /tmp/tmp-qupzilla-release/QupZilla.app

hdiutil detach /tmp/tmp-qupzilla-release

echo "Creating final compressed(bz2) DMG…"
rm $BUNDLE_PATH/QupZilla.dmg
hdiutil convert /tmp/tmp-qupzilla.dmg -format UDBZ -o $BUNDLE_PATH/QupZilla.dmg

echo "Removing temp files…"
rm /tmp/tmp-qupzilla.dmg
exit
