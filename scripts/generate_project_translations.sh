#!/bin/bash
#
# Generates list of all translations for qmake .pro and .qrc files

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

    echo -e "\n"

    for translation in $dir/*.ts
    do
        [[ "$translation" == *empty.ts ]] && continue
        echo "        <file>locale/`echo $translation | awk 'BEGIN{FS="/"}{print substr($6,0,length($6)-3)}'`.qm</file>"
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

