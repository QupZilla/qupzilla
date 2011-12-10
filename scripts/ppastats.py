#!/usr/bin/python
#usage python ppastats.py PPATEAM (ex: webupd8team) PPA (ex: gthumb) DIST (Ubuntu version eg maverick) ARCH (ubuntu arch eg i386 or amd64)

#example - highest downloaded file: python ppastats.py webupd8team y-ppa-manager maverick amd64 | tr '\t' ',' | cut -d ',' -f3 | sort -gr

import sys
from launchpadlib.launchpad import Launchpad

PPAOWNER = sys.argv[1]
PPA = sys.argv[2]
desired_dist_and_arch = 'https://api.launchpad.net/devel/ubuntu/' + sys.argv[3] + '/' + sys.argv[4]



cachedir = "~/.launchpadlib/cache/"
lp_ = Launchpad.login_anonymously('ppastats', 'edge', cachedir, version='devel')
owner = lp_.people[PPAOWNER]
archive = owner.getPPAByName(name=PPA)

for individualarchive in archive.getPublishedBinaries(status='Published',distro_arch_series=desired_dist_and_arch):

       x = individualarchive.getDownloadCount()
       if x > 0:
              print individualarchive.binary_package_name + "\t" + individualarchive.binary_package_version + "\t" + str(individualarchive.getDownloadCount())
       elif x < 1:
              print '0'
