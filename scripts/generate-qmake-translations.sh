#!/bin/bash
#
# Generates list of all translations for qmake .pro

# Plugins translations
PLUGINS="`ls -d ../src/plugins/*/translations`"

for dir in $PLUGINS
do
    echo -e "$dir\n"
    echo "TRANSLATIONS += \\"

    for translation in $dir/*.ts
    do
        [[ "$translation" == *empty.ts ]] && continue
        echo "    `echo $translation | awk 'BEGIN{FS="/"}{printf "%s/%s",$5,$6}'` \\"
    done

    echo -e "\n\n"
done

# App translations
echo -e "../translations\n"
echo "TRANSLATIONS += \\"

for translation in ../translations/*.ts
do
    [[ "$translation" == *empty.ts ]] && continue
    echo "    `echo $translation | awk 'BEGIN{FS="/"}{printf "$$PWD/%s",$3}'` \\"
done

echo -e "\n\n"

