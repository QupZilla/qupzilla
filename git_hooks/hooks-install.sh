#!/bin/bash
#
# Install hooks into .git/hooks directory
#

cp post-commit ../.git/hooks/post-commit
cp post-checkout ../.git/hooks/post-checkout
cp post-merge ../.git/hooks/post-merge
