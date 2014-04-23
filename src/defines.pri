DESTDIR = $$PWD/../bin
OBJECTS_DIR = $$PWD/../build
MOC_DIR = $$PWD/../build
RCC_DIR = $$PWD/../build
UI_DIR = $$PWD/../build

# workaround for #849: see https://bugreports.qt-project.org/browse/QTBUG-23196
mocinclude.CONFIG *= fix_target

unix: VERSION = 1.6.5

# Please read BUILD information #
#DEFINES *= PORTABLE_BUILD

win32-msvc* {
    DEFINES *= W7API
    LIBS += User32.lib Ole32.lib Shell32.lib ShlWapi.lib Gdi32.lib ComCtl32.lib
}

# QtDBus not available on Mac
mac: DEFINES *= DISABLE_DBUS

# Check for pkg-config availability
!mac:unix:system(pkg-config --version > /dev/null) {
    isEqual(QT_MAJOR_VERSION, 5) {
        MODNAME = Qt5WebKitWidgets
    }
    else {
        MODNAME = QtWebKit
    }

    QTWEBKIT_VERSION = $$system(PKG_CONFIG_PATH="$$[QT_INSTALL_LIBS]/pkgconfig" pkg-config --modversion $$MODNAME)
    QTWEBKIT_VERSION_MAJOR = $$section(QTWEBKIT_VERSION, ".", 0, 0)
    QTWEBKIT_VERSION_MINOR = $$section(QTWEBKIT_VERSION, ".", 1, 1)

    isEqual(QT_MAJOR_VERSION, 5) {
        greaterThan(QTWEBKIT_VERSION_MAJOR, 4) {
            DEFINES *= USE_QTWEBKIT_2_2 USE_QTWEBKIT_2_3
        }
    }
    else { # Qt 4
        greaterThan(QTWEBKIT_VERSION_MAJOR, 3):greaterThan(QTWEBKIT_VERSION_MINOR, 8) {
            DEFINES *= USE_QTWEBKIT_2_2
        }

        greaterThan(QTWEBKIT_VERSION_MAJOR, 3):greaterThan(QTWEBKIT_VERSION_MINOR, 9) {
            DEFINES *= USE_QTWEBKIT_2_3
        }
    }
}
else {
    isEqual(QT_VERSION, 4.8.0)|greaterThan(QT_VERSION, 4.8.0) {
        DEFINES *= USE_QTWEBKIT_2_2
    }

    isEqual(QT_MAJOR_VERSION, 5) {
        DEFINES *= USE_QTWEBKIT_2_2 USE_QTWEBKIT_2_3
    }
}

DEFINES *= QT_NO_URL_CAST_FROM_STRING
DEFINES *= QT_USE_QSTRINGBUILDER

CONFIG(debug, debug|release): DEFINES *= QUPZILLA_DEBUG_BUILD

d_no_system_datapath = $$(NO_SYSTEM_DATAPATH)
d_use_webgl = $$(USE_WEBGL)
d_w7api = $$(W7API)
d_kde = $$(KDE) # Backwards compatibility
d_kde_integration = $$(KDE_INTEGRATION)
d_gnome_integration = $$(GNOME_INTEGRATION)
d_nox11 = $$(NO_X11)
d_portable = $$(PORTABLE_BUILD)
d_nonblock_dialogs = $$(NONBLOCK_JS_DIALOGS)
d_use_qtwebkit_2_2 = $$(USE_QTWEBKIT_2_2)
d_use_lib_path = $$(USE_LIBPATH)
d_disable_dbus = $$(DISABLE_DBUS)
d_disable_updates_check = $$(DISABLE_UPDATES_CHECK)

equals(d_no_system_datapath, "true") { DEFINES *= NO_SYSTEM_DATAPATH }
equals(d_use_webgl, "true") { DEFINES *= USE_WEBGL }
win32-msvc*:equals(d_w7api, "true") { DEFINES *= W7API }
equals(d_kde, "true") { DEFINES *= KDE_INTEGRATION }
equals(d_kde_integration, "true") { DEFINES *= KDE_INTEGRATION }
equals(d_gnome_integration, "true") { DEFINES *= GNOME_INTEGRATION }
equals(d_nox11, "true") { DEFINES *= NO_X11 }
equals(d_portable, "true") { DEFINES *= PORTABLE_BUILD }
equals(d_nonblock_dialogs, "true") { DEFINES *= NONBLOCK_JS_DIALOGS }
equals(d_use_qtwebkit_2_2, "true") { DEFINES *= USE_QTWEBKIT_2_2 }
equals(d_disable_dbus, "true") { DEFINES *= DISABLE_DBUS }
equals(d_disable_updates_check, "true") { DEFINES *= DISABLE_UPDATES_CHECK }

!mac:unix {
    x86libpath = /usr/lib/i386-linux-gnu
    x64libpath = /usr/lib/x86_64-linux-gnu
    system_lib_path = /usr/lib

    # QMAKE_HOST.arch is empty on x86
    contains(QMAKE_HOST.arch, x86_64) {
        exists($$x64libpath): system_lib_path = $$x64libpath
    }
    else {
        exists($$x86libpath): system_lib_path = $$x86libpath
    }

    d_prefix = $$(QUPZILLA_PREFIX)
    binary_folder = /usr/bin
    library_folder = $$system_lib_path
    data_folder = /usr/share/qupzilla
    launcher_folder = /usr/share/applications
    icon_folder = /usr/share/pixmaps
    hicolor_folder = /usr/share/icons/hicolor

    !equals(d_prefix, "") {
        binary_folder = "$$d_prefix"/bin
        library_folder = "$$d_prefix"/lib
        data_folder = "$$d_prefix"/share/qupzilla
        launcher_folder = "$$d_prefix"/share/applications
        icon_folder = "$$d_prefix"/share/pixmaps
        hicolor_folder = "$$d_prefix"/share/icons/hicolor
    }

    !equals(d_use_lib_path, ""):library_folder = $$d_use_lib_path

    DEFINES *= USE_LIBPATH=\\\"""$$library_folder"\\\""
    DEFINES *= USE_DATADIR=\\\"""$$data_folder"\\\""

    # Git revision
    rev = $$system(cd ../ && sh $$PWD/../scripts/getrevision.sh)
    !equals(rev, ""): DEFINES *= GIT_REVISION=\\\"""$$rev"\\\""

    # Define QZ_WS_X11 even with Qt5 (but only when building for X11)
    !contains(DEFINES, NO_X11) DEFINES *= QZ_WS_X11
}

isEmpty(QMAKE_LRELEASE) {
    win32|os2:QMAKE_LRELEASE = $$[QT_INSTALL_BINS]\\lrelease.exe
    else:QMAKE_LRELEASE = $$[QT_INSTALL_BINS]/lrelease
    unix {
        !exists($$QMAKE_LRELEASE) { QMAKE_LRELEASE = lrelease-qt4 }
    } else {
        !exists($$QMAKE_LRELEASE) { QMAKE_LRELEASE = lrelease }
    }
}

isEmpty(QMAKE_LFLAGS_RPATH) {
    QMAKE_LFLAGS_RPATH = -Wl,-rpath,
}
