#!/bin/bash
#
# Requirements:
#   astyle > =2.02 (patched with foreach support)
#   normalize (Qt tool to normalize all signal/slots format)
#

function format_sources {
    astyle --indent=spaces=4 --style=1tbs \
       --indent-labels --pad-oper --unpad-paren --pad-header \
       --convert-tabs --indent-preprocessor --break-closing-brackets \
       --align-pointer=type --align-reference=name \
       `find -type f -name '*.cpp'` | grep 'Formatted'

    find . -name "*.orig" -print0 | xargs -0 rm -rf
}

function format_headers {
    astyle --indent=spaces=4 --style=linux \
       --indent-labels --pad-oper --unpad-paren --pad-header \
       --keep-one-line-statements --keep-one-line-blocks \
       --indent-preprocessor --convert-tabs \
       --align-pointer=type --align-reference=name \
       `find -type f -name '*.h'` | grep 'Formatted'

    find . -name "*.orig" -print0 | xargs -0 rm -rf
}

cd ../src
echo "Running astyle for *.cpp ..."
format_sources

echo "Running astyle for *.h ..."
format_headers

echo "Running normalize ..."
normalize --modify .

exit 0
