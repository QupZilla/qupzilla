#!/bin/bash
#git show-ref refs/heads/master | cut -d " " -f 1 | cut -c 1-10

cd ..
if [ -e "git_revision" ]; then
    cat git_revision | cut -c 1-10;
fi

exit;
