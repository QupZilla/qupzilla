DESTDIR = $$PWD/../bin
QZ_DESTDIR = $$DESTDIR
OBJECTS_DIR = $$PWD/../build
MOC_DIR = $$PWD/../build
RCC_DIR = $$PWD/../build
UI_DIR = $$PWD/../build

# workaround for #849: see https://bugreports.qt-project.org/browse/QTBUG-23196
mocinclude.CONFIG *= fix_target

QZ_VERSION = 1.8.6
unix: VERSION = $$QZ_VERSION
DEFINES *= QUPZILLA_VERSION=\\\"""$$QZ_VERSION"\\\""

d_no_system_datapath = $$(NO_SYSTEM_DATAPATH)
d_use_webgl = $$(USE_WEBGL)
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
d_debug_build = $$(DEBUG_BUILD)
d_prefix = $$(QUPZILLA_PREFIX)

equals(d_no_system_datapath, "true") { DEFINES *= NO_SYSTEM_DATAPATH }
equals(d_use_webgl, "true") { DEFINES *= USE_WEBGL }
equals(d_kde, "true") { DEFINES *= KDE_INTEGRATION }
equals(d_kde_integration, "true") { DEFINES *= KDE_INTEGRATION }
equals(d_gnome_integration, "true") { DEFINES *= GNOME_INTEGRATION }
equals(d_nox11, "true") { DEFINES *= NO_X11 }
equals(d_portable, "true") { DEFINES *= PORTABLE_BUILD }
equals(d_nonblock_dialogs, "true") { DEFINES *= NONBLOCK_JS_DIALOGS }
equals(d_use_qtwebkit_2_2, "true") { DEFINES *= USE_QTWEBKIT_2_2 }
equals(d_disable_dbus, "true") { DEFINES *= DISABLE_DBUS }
equals(d_disable_updates_check, "true") { DEFINES *= DISABLE_UPDATES_CHECK }
equals(d_debug_build, "true") { CONFIG += debug }

DEFINES *= QT_NO_URL_CAST_FROM_STRING
DEFINES *= QT_USE_QSTRINGBUILDER

CONFIG(debug, debug|release): DEFINES *= QUPZILLA_DEBUG_BUILD


win32-msvc* {
    DEFINES *= W7API
    LIBS += User32.lib Ole32.lib Shell32.lib ShlWapi.lib Gdi32.lib ComCtl32.lib
}

# QtDBus not available on Mac
mac: DEFINES *= DISABLE_DBUS

haiku-* {
    DEFINES *= DISABLE_DBUS
    DEFINES *= NO_SYSTEM_DATAPATH
    DEFINES *= NO_X11
}

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
            # There is one Qt5WebKitWidgets version now, which has same features as QtWebKit 2.3
            DEFINES *= USE_QTWEBKIT_2_2 USE_QTWEBKIT_2_3
        }
    }
    else { # Qt 4
        equals(QTWEBKIT_VERSION_MAJOR, 4):greaterThan(QTWEBKIT_VERSION_MINOR, 8) {
            # 4.9.x = QtWebKit 2.2
            DEFINES *= USE_QTWEBKIT_2_2
        }

        equals(QTWEBKIT_VERSION_MAJOR, 4):greaterThan(QTWEBKIT_VERSION_MINOR, 9) {
            # 4.10.x = QtWebKit 2.3
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


!mac:unix {
    binary_folder = /usr/bin
    library_folder = /usr/lib
    arch_lib_path = /usr/lib/$${QT_ARCH}-linux-gnu
    exists($$arch_lib_path): library_folder = $$arch_lib_path
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

    # Try to use lrelease from PATH
    unix:!exists($$QMAKE_LRELEASE) {
        isEqual(QT_MAJOR_VERSION, 4): QMAKE_LRELEASE = lrelease-qt4
        else: QMAKE_LRELEASE = lrelease
    }
}

isEmpty(QMAKE_LFLAGS_RPATH) {
    QMAKE_LFLAGS_RPATH = -Wl,-rpath,
}
