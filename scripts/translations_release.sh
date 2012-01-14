#!/bin/bash
# run lrelease, .qm files will be located in ../translations/
lrelease ../src/src.pro
# removing empty.qm
rm ../translations/empty.qm
# moving .qm files into ..bin/locale/
mv ../translations/*.qm ../bin/locale/

read -p "Press [ENTER] to close terminal"
exit
