#!/bin/bash
# It will probably work only for Debian based distros

cat /etc/ssl/certs/*.pem > ../src/lib/data/data/ca-bundle.crt

exit 0
