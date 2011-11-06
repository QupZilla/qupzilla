#!/bin/bash
cat /usr/share/ca-certificates/*/*.crt > ../other/ca-bundle.crt

read -p "Press [ENTER] to close terminal"
exit
