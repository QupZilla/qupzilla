#!/bin/bash

/home/david/Programování/qtsdk-2010.05/qt/bin/lupdate /home/david/Programování/Qt\ C++/QupZilla/plugins/TestPlugin/src/TestPlugin.pro -no-obsolete -ts /home/david/Programování/Qt\ C++/QupZilla/plugins/TestPlugin/cs_CZ.ts

/home/david/Programování/qtsdk-2010.05/qt/bin/lupdate /home/david/Programování/Qt\ C++/QupZilla/plugins/TestPlugin/src/TestPlugin.pro -no-obsolete -ts /home/david/Programování/Qt\ C++/QupZilla/plugins/TestPlugin/sk_SK.ts

read -p "Press [ENTER] to close terminal"
exit
