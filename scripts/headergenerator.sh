#!/bin/bash
## headers.tar.gz generator
cd ../src
mkdir includes
find . -name '*.h' -exec cp {} includes/ \;
tar -cvzf headers.tar.gz includes/
rm -r includes/
mv headers.tar.gz ../headers.tar.gz

read -p "Press [ENTER] to close terminal"
exit
