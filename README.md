QupZilla Web Browser
----------------------------------------------------------------------------------------

[![Build Status](https://travis-ci.org/QupZilla/qupzilla.svg?branch=v1.8)](https://travis-ci.org/QupZilla/qupzilla)  
Homepage: [http://www.qupzilla.com](http://www.qupzilla.com)  
Blog: [http://blog.qupzilla.com](http://blog.qupzilla.com)  
IRC: `#qupzilla` at `irc.freenode.net`  
Translations: [https://www.transifex.com](https://www.transifex.com/projects/p/qupzilla)  

About QupZilla
----------------------------------------------------------------------------------------

QupZilla is a new and very fast QtWebKit browser. It aims to be a lightweight web browser
available through all major platforms. This project has been originally started only
for educational purposes. But from its start, QupZilla has grown into a feature-rich browser.

QupZilla has all standard functions you expect from a web browser. It includes bookmarks,
history (both also in sidebar) and tabs. Above that, you can manage RSS feeds with an included
RSS reader, block ads with a built-in AdBlock plugin, block Flash content with Click2Flash
and edit the local CA Certificates database with an SSL Manager.

QupZilla's main aim is to be a very fast and very stable QtWebKit browser available to everyone.
There are already a lot of QtWebKit browsers available, but they are either bound to the KDE
environment (rekonq), are not actively developed or very unstable and miss important
features. But there is missing a multi-platform, modern and actively developed browser. QupZilla
is trying to fill this gap by providing a very stable browsing experience.

History
----------------------------------------------------------------------------------------

The very first version of QupZilla has been released in December 2010 and it was written
in Python with PyQt4 bindings. After a few versions, QupZilla has been completely rewritten
in C++ with the Qt Framework. The Windows version of QupZilla was compiled using MingW, but due to
a huge problem with Flash, it is now compiled with Microsoft Visual C++ Compiler 2008.
First public release was 1.0.0-b4.

Compiling
----------------------------------------------------------------------------------------

Before you start compiling, make sure that you have installed the Qt (>=4.7) development libraries
and you have read the [BUILDING](https://github.com/QupZilla/qupzilla/blob/master/BUILDING) information.

**Linux**

 * OpenSSL (libcrypto) is required
 * pkg-config is recommended (to correctly detect versions of QtWebKit)
 * pkg-config is required for Hunspell spellcheck
 * Hunspell development package for spellcheck
 * QtWebKit 2.3 is needed to have support for spellcheck

**Windows**
 * OpenSSL (libeay32) is required
 * QtWebKit 2.3 is required
 * Hunspell library is required for spellcheck

Then you can start compiling by running this commands:

    $ qmake
    $ make

After a successful compilation the executable binary can be found in the bin/ directory.
On Fedora and possibly other Linux distributions you need to replace `qmake` with `qmake-qt4` or `qmake-qt5` since `qmake` is for Qt3.

On Linux/Unix: To install QupZilla, run this command: (it may be necessary to run it as root)

    $ make install

On Mac OS X: To deploy QupZilla in dmg image, run this command:

    $ ./mac/macdeploy.sh full-path-to-macdeployqt

You need to specify path to `macdeployqt` only if it is not in PATH.

Current version
----------------------------------------------------------------------------------------

The current stable version of QupZilla is 1.8.7. You can download precompiled packages
and the sources from the download section at [homepage](http://www.qupzilla.com/download).
However, if you want the latest revision, just take the latest code snapshot either by
downloading a tarball or running:

    $ git clone git://github.com/QupZilla/qupzilla.git

If you are using Ubuntu, you can download QupZilla from PPA:

    $ sudo add-apt-repository ppa:nowrep/qupzilla
    $ sudo apt-get update
    $ sudo apt-get install qupzilla

    for development version:

    $ sudo apt-get install qupzilla-next

FAQ and Changelog
----------------------------------------------------------------------------------------

If you are experiencing some sort of problem, please read the FAQ before you open an issue.

[FAQ](https://github.com/QupZilla/qupzilla/wiki/FAQ) | [Changelog](https://github.com/QupZilla/qupzilla/blob/master/CHANGELOG) | [Bug Reports](https://github.com/QupZilla/qupzilla/wiki/Bug-Reports)
