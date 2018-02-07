General
----------------------------------------------------------------------------------

  If you can, you should use precompiled packages for your distribution.
  But if you cannot use them, or they are not available, please read
  this information before compiling.
  After your binary is successfully compiled, you need to copy bin/ folder
  from git to specific directory by your system you compiled for.
  On Linux, you can easily do it by running make install.
  If you are unsure where is the right place, you can check it directly from
  QupZilla by clicking from Help Menu on Configuration Information, then in
  Path section.

  You may want to build QupZilla with debugging symbols (for generating
  backtrace of crash). You can enable debug build with:

               qmake CONFIG+=debug -r

  QupZilla requires Qt (>= 5.9) and QtWebEngine (at least version included in Qt 5.9)

Microsoft Windows
----------------------------------------------------------------------------------

  You need Microsoft Visual C++ Compiler, Qt Libraries 5.9 or higher and openssl
  libraries. in order to build QupZilla.

Linux / Unix
----------------------------------------------------------------------------------

  You need to have Qt 5 (>= 5.9) with QtWebEngine.
  Next compulsory requirement is OpenSSL (libcrypto). xcb libraries are also
  required unless you specify NO_X11 build option.

  To build KWallet plugin, you need:
     - KF5 KWallet
     - set KDE_INTEGRATION build flag

  To build Gnome-Keyring plugin, you need
     - libgnome-keyring-dev installed
     - set GNOME_INTEGRATION build flag

  For debug build, gdb is required by qmake.

MAC OS X
----------------------------------------------------------------------------------

  You need to have Xcode from the Apple App Store installed in Applications, [Command Line Tools for the same Xcode version](https://developer.apple.com/) may be included depending on the version,
  [Homebrew](http://brew.sh/), and `$ brew install openssl` for openssl.
  Next compulsory requirement is Qt 5 (>= 5.9) with QtWebEngine.
  After successful compilation, you need to build the application bundle. You will do it with following command:

    $ make bundle

Available Defines
----------------------------------------------------------------------------------

  You can set define directly in file (src/defines.pri)
  or set environment variable by

    $ export NAME="value"

 General:

    PORTABLE_BUILD        QupZilla won't write any data outside of path of execution.
                          It will also disable plugins by default.
                          (disabled by default)

                          example:
                          $ export PORTABLE_BUILD="true"

 Windows specific defines:

     W7API                Enable Windows 7 API support
                          Requires linking against libraries from Microsoft Visual C++
                          Compiler 2010
                          (enabled by default)

 Linux / Unix specific defines:

     NO_X11               Disable all X11 calls.
                          Enable this when building for Wayland-only.
                          All X11 calls are guarded by runtime X11 platform check
                          even without this option.

                          example:
                          $ export NO_X11="true"

     KDE_INTEGRATION      Enable KDE integration.
                          Currently it enables building of KWallet Password plugin,
                          which provides support for storing passwords in KWallet.

                          example:
                          $ export KDE_INTEGRATION="true"

     GNOME_INTEGRATION    Enable Gnome integration.
                          Currently it enables building of Gnome-Keyring Password plugin,
                          which provides support for storing passwords in Gnome-Keyring.

                          example:
                          $ export GNOME_INTEGRATION="true"

     USE_LIBPATH          By default, /usr/lib/ is used for libQupZilla and /usr/lib/qupzilla
                          for plugins.
                          You can change it by setting this define.

                          example:
                          $ export USE_LIBPATH="/usr/lib64"

     NO_SYSTEM_DATAPATH   By default, QupZilla is using /usr/share/qupzilla/ path
                          for storing themes and translations.
                          By setting this define, QupZilla will use path of execution.
                          (disabled by default)

                          example:
                          $ export NO_SYSTEM_DATAPATH="true"

     QUPZILLA_PREFIX      You can define different prefix.
                          QupZilla binary will then be moved to PREFIX/bin/, use
                          PREFIX/share/qupzilla/ as datadir, PREFIX/share/applications for
                          desktop launcher and PREFIX/share/pixmaps for icon.
                          (default prefix is "/usr")

                          example:
                          $ export QUPZILLA_PREFIX="/usr"

     SHARE_FOLDER         You can define the path of the share folder, i.e. /usr/share
                          QupZilla will then use SHARE_FOLDER/qupzilla as datadir,
                          SHARE_FOLDER/applications for desktop launcher and
                          SHARE_FOLDER/pixmaps for the icon. By default it is not defined
                          and files will be installed as described above.
                          (default share folder is "/usr/share")

                          example:
                          $ export SHARE_FOLDER="/usr/share"

     DISABLE_DBUS         Build without QtDBus module. Native desktop notifications
                          will be disabled.

                          example:
                          $ export DISABLE_DBUS="true"

