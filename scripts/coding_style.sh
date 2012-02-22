#!/bin/bash
#
# Requirements:
#   astyle >=2.02
#

function format_sources {
    astyle --indent=spaces=4 --style=1tbs \
       --indent-labels --pad-oper --unpad-paren --pad-header \
       --convert-tabs --indent-preprocessor --break-closing-brackets \
       --align-pointer=type --align-reference=name \
       `find -type f -name '*.cpp'`
       
    rm */*.orig
}

function format_headers {
    astyle --indent=spaces=4 --style=linux \
       --indent-labels --pad-oper --unpad-paren --pad-header \
       --keep-one-line-statements --keep-one-line-blocks \
       --indent-preprocessor --convert-tabs \
       --align-pointer=type --align-reference=name \
       `find -type f -name '*.h'`
       
    rm */*.orig
}

cd ../src
echo "running astyle for *.cpp ..."
format_sources

echo "running astyle for *.h ..."
format_headers

echo "running astyle for plugins ..."
cd ../plugins
format_sources
format_headers
       
read -p "Press [ENTER] to close terminal"
exit
