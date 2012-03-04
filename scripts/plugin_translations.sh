#!/bin/bash

lupdate ../src/plugins/TestPlugin/TestPlugin.pro -no-obsolete

lupdate ../src/plugins/MouseGestures/MouseGestures.pro -no-obsolete

lupdate ../src/plugins/AccessKeysNavigation/AccessKeysNavigation.pro -no-obsolete

read -p "Press [ENTER] to close terminal"
exit
