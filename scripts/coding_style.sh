#!/bin/bash
#
# Requirements:
#   astyle > =2.02 (patched with foreach support)
#   normalize (Qt tool to normalize all signal/slots format)
#

OPTIONS="--indent=spaces=4 --style=linux
         --pad-oper --unpad-paren --pad-header --convert-tabs
         --indent-preprocessor --break-closing-brackets
         --align-pointer=type --align-reference=name
         --suffix=none --formatted"

function format_sources {
    astyle $OPTIONS \
        `find -type f \( -name '*.cpp' -not -name 'moc_*.cpp' \)`
}

function format_headers {
    astyle $OPTIONS --keep-one-line-statements --keep-one-line-blocks \
       `find -type f -name '*.h'`
}

cd ../src

echo "Running astyle for *.cpp ..."
format_sources

echo "Running astyle for *.h ..."
format_headers

echo "Running normalize ..."
normalize --modify .

exit 0
