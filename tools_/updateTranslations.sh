#!/bin/bash

## circular inclusions workaround - we comment that buggy line
sed -i 's/include(3rdparty/##temp/g' /home/david/Programování/Qt\ C++/QupZilla/src/QupZilla.pro

/home/david/Programování/qtsdk-2010.05/qt/bin/lupdate /home/david/Programování/Qt\ C++/QupZilla/src/QupZilla.pro -no-obsolete -ts /home/david/Programování/Qt\ C++/QupZilla/translations/cs_CZ.ts

/home/david/Programování/qtsdk-2010.05/qt/bin/lupdate /home/david/Programování/Qt\ C++/QupZilla/src/QupZilla.pro -no-obsolete -ts /home/david/Programování/Qt\ C++/QupZilla/translations/sk_SK.ts

## uncomment it now
sed -i 's/##temp/include(3rdparty/g' /home/david/Programování/Qt\ C++/QupZilla/src/QupZilla.pro

read -p "Press [ENTER] to close terminal"
exit
