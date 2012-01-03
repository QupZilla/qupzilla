#!/bin/bash
# It will probably work only for Debian based distros

cat /usr/share/ca-certificates/*/*.crt > ../src/data/data/ca-bundle.crt
cat /etc/ssl/certs/*.pem >> ../src/data/data/ca-bundle.crt

read -p "Press [ENTER] to close terminal"
exit
