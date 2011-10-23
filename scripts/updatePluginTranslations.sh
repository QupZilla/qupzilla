#!/bin/bash

lupdate ../plugins/TestPlugin/TestPlugin.pro -no-obsolete -ts ../plugins/TestPlugin/cs_CZ.ts

lupdate ../plugins/TestPlugin/TestPlugin.pro -no-obsolete -ts ../plugins/TestPlugin/sk_SK.ts

read -p "Press [ENTER] to close terminal"
exit
