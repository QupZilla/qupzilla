#!/bin/bash

REV=""

if [ -e "/usr/bin/git" ] && ([ -e ".git" ] || [ -e "../.git" ]); then
    REV=`git rev-parse HEAD`
elif [ -e "git_revision" ]; then
    REV=`cat git_revision`
fi

case $1 in
    long)
        echo $REV
        ;;

    short|*)
        echo $REV | cut -c 1-10
        ;;
esac

exit 0
