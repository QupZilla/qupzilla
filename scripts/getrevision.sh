#!/bin/bash
#git show-ref refs/heads/master | cut -d " " -f 1 | cut -c 1-10

if [ -e "/usr/bin/git" ] &&  [ -e ".git" ]; then
    git show-ref refs/heads/master | cut -d " " -f 1 | cut -c 1-10;
elif [ -e "git_revision" ]; then
    cat git_revision | cut -c 1-10;
fi

exit;
