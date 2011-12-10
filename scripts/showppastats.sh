#!/bin/bash
TEAM="nowrep"
PACKAGE="qupzilla"
echo ""
echo "Launchpad PPA Statistics"
echo "Team: $TEAM"
echo "Package: $PACKAGE"
echo ""
echo "Oneiric:   i386: " $(python ppastats.py $TEAM $PACKAGE oneiric i386|tail -c -2);
echo "          amd64: " $(python ppastats.py $TEAM $PACKAGE oneiric amd64|tail -c -2);
echo ""
echo "Natty:     i386: " $(python ppastats.py $TEAM $PACKAGE natty i386|tail -c -2);
echo "          amd64: " $(python ppastats.py $TEAM $PACKAGE natty amd64|tail -c -2);
echo ""
echo "Maverick:  i386: " $(python ppastats.py $TEAM $PACKAGE maverick i386|tail -c -2);
echo "          amd64: " $(python ppastats.py $TEAM $PACKAGE maverick amd64|tail -c -2);
echo ""
read -p "Press [ENTER] to close terminal"
exit
