QupZilla Web Browser - http://www.qupzilla.co.cc (http://qupzilla.blogspot.com/)

About QupZilla
----------------------------------------------------------------------------------------

QupZilla is a new, very fast QtWebKit browser. It aims to be lightweight web browser
available through all major platforms. This project has been originally started only
for educational purposes, but from its start, QupZilla grown into a rich feature browser.

QupZilla has all standard functions you expect from web browser. It includes bookmarks,
history (both also in sidebar) and tabs. Above that, you can read RSS feeds with included
RSS reader, block ads with builtin AdBlock plugin, block Flash content with Click2Flash
or edit CA Certificates database with SSL Manager.

QupZilla's main aim is to be very fast and very stable QtWebKit browser available to everyone.
There is already a lot of QtWebKit browsers available, but they are either bound to KDE
environment (rekonq), not actively developed or very unstable and missing important
features. But there is missing multiplatform, modern and actively developped browser. QupZilla 
is trying to fill this gap by providing very stable browsing experience.

History
----------------------------------------------------------------------------------------

The very first version of QupZilla has been released in Decemeber 2010 and it was written
in Python with PyQt4 bindings. After few versions, QupZilla has been completely rewritten
in C++ with Qt Framework. Windows version of QupZilla was compiled using MingW, but due to
huge problem with Flash, it is now compiled with Microsoft Visual C++ Compiler 2008.
First public release was 1.0.0-b4.

Compiling
----------------------------------------------------------------------------------------

Before compiling, make sure you have have installed Qt (>=4.7) development libraries and
you read BUILDING informations.

Then you can start compile by running this commands:

    $ cd src/
    $ qmake
    $ make

The executable binary can now be found in bin/ directory.

To install QupZilla, run this command: (it may be neccessary to run as root)

    $ make install
    
Current version
----------------------------------------------------------------------------------------

Current released version of QupZilla is 1.0.0-rc1, you can download precompiled packages
and source from download section.
However, if you want latest revision, just take the latest code snapshot either by
downloading a tarball or running:

    $ git clone git://github.com/nowrep/QupZilla.git
    
FAQ and Changelog
----------------------------------------------------------------------------------------

If you are experiencing some sort of issue, before you open an issue, please read FAQ.

FAQ:       https://github.com/nowrep/QupZilla/blob/master/FAQ

Changelog: https://github.com/nowrep/QupZilla/wiki/Changelog
