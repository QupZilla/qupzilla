#!/bin/bash

REV=""

if [ -e "/usr/bin/git" ] && ([ -e ".git" ] || [ -e "../.git" ]); then
    REV=`git show-ref refs/heads/v1.8 | cut -d " " -f 1`
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
