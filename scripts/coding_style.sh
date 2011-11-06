#!/bin/bash
#
# Requirements:
#   astyle >=2.02
#

echo "running astyle for *.cpp ..."

cd ../src

astyle --indent=spaces=4 --style=1tbs \
       --indent-labels --pad-oper --unpad-paren --pad-header \
       --convert-tabs --indent-preprocessor --break-closing-brackets \
       --align-pointer=type --align-reference=name \
       `find -type f -name '*.cpp'`

echo "running astyle for *.h ..."

astyle --indent=spaces=4 --style=linux \
       --indent-labels --pad-oper --unpad-paren --pad-header \
       --keep-one-line-statements --keep-one-line-blocks \
       --indent-preprocessor --convert-tabs \
       --align-pointer=type --align-reference=name \
       `find -type f -name '*.h'`
       
rm */*.orig
       
read -p "Press [ENTER] to close terminal"
exit
