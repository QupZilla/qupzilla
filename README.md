QupZilla Web Browser
----------------------------------------------------------------------------------------

[![Build Status](https://travis-ci.org/QupZilla/qupzilla.svg?branch=master)](https://travis-ci.org/QupZilla/qupzilla)  
Homepage: [http://www.qupzilla.com](http://www.qupzilla.com)  
Blog: [http://blog.qupzilla.com](http://blog.qupzilla.com)  
IRC: `#qupzilla` at `irc.freenode.net`  
Translations: [https://www.transifex.com](https://www.transifex.com/projects/p/qupzilla)

![](https://github.com/QupZilla/qupzilla/blob/master/src/lib/data/icons/other/about.png?raw=true)   

QupZilla is a new and very fast QtWebEngine browser. It aims to be a lightweight web browser
available through all major platforms. This project has been originally started only
for educational purposes. But from its start, QupZilla has grown into a feature-rich browser.

QupZilla has all standard functions you expect from a web browser. It includes bookmarks,
history (both also in sidebar) and tabs. Above that, it has by default enabled blocking ads
with a built-in AdBlock plugin.

History
----------------------------------------------------------------------------------------

The very first version of QupZilla has been released in December 2010 and it was written
in Python with PyQt4 bindings. After a few versions, QupZilla has been completely rewritten
in C++ with the Qt Framework. First public release was 1.0.0-b4.

Until version 2.0, QupZilla was using QtWebKit. QtWebKit is now deprecated and new versions
are using QtWebEngine.

Compiling
----------------------------------------------------------------------------------------

Before you start compiling, make sure that you have installed the Qt (>= 5.7) development libraries
and you have read the [BUILDING.md](https://github.com/QupZilla/qupzilla/blob/master/BUILDING.md) information.

**Linux**

 * OpenSSL (libcrypto) is required
 * xcb libraries when building without NO_X11

**Windows**
 * OpenSSL (libeay32) is required

Then you can start compiling by running this commands:

    $ qmake
    $ make

After a successful compilation the executable binary can be found in the bin/ directory.
On Fedora and possibly other Linux distributions you need to replace `qmake` with `qmake-qt5`.

On Linux/Unix: To install QupZilla, run this command: (it may be necessary to run it as root)

    $ make install

On Mac OS X: To deploy QupZilla in dmg image, run this command:

    $ ./mac/macdeploy.sh full-path-to-macdeployqt

You need to specify path to `macdeployqt` only if it is not in PATH.

Current version
----------------------------------------------------------------------------------------

The current stable version of QupZilla is 2.1.2. You can download precompiled packages
and the sources from the download section at [homepage](http://www.qupzilla.com/download).
However, if you want the latest revision, just take the latest code snapshot either by
downloading a tarball or running:

    $ git clone git://github.com/QupZilla/qupzilla.git

FAQ and Changelog
----------------------------------------------------------------------------------------

If you are experiencing some sort of problem, please read the FAQ before you open an issue.

[FAQ](https://github.com/QupZilla/qupzilla/wiki/FAQ) | [Changelog](https://github.com/QupZilla/qupzilla/blob/master/CHANGELOG) | [Bug Reports](https://github.com/QupZilla/qupzilla/wiki/Bug-Reports)
