DESTDIR = $$PWD/../bin
QZ_DESTDIR = $$DESTDIR
OBJECTS_DIR = $$PWD/../build
MOC_DIR = $$PWD/../build
RCC_DIR = $$PWD/../build
UI_DIR = $$PWD/../build

# workaround for #849: see https://bugreports.qt-project.org/browse/QTBUG-23196
mocinclude.CONFIG *= fix_target

QZ_VERSION = 2.2.6
unix: VERSION = $$QZ_VERSION
DEFINES *= QUPZILLA_VERSION=\\\"""$$QZ_VERSION"\\\""

d_no_system_datapath = $$(NO_SYSTEM_DATAPATH)
d_kde_integration = $$(KDE_INTEGRATION)
d_gnome_integration = $$(GNOME_INTEGRATION)
d_nox11 = $$(NO_X11)
d_portable = $$(PORTABLE_BUILD)
d_use_lib_path = $$(USE_LIBPATH)
d_disable_dbus = $$(DISABLE_DBUS)
d_disable_updates_check = $$(DISABLE_UPDATES_CHECK)
d_debug_build = $$(DEBUG_BUILD)
d_prefix = $$(QUPZILLA_PREFIX)
d_share = $$(SHARE_FOLDER)

equals(d_no_system_datapath, "true") { DEFINES *= NO_SYSTEM_DATAPATH }
equals(d_kde_integration, "true") { DEFINES *= KDE_INTEGRATION }
equals(d_gnome_integration, "true") { DEFINES *= GNOME_INTEGRATION }
equals(d_nox11, "true") { DEFINES *= NO_X11 }
equals(d_portable, "true") { DEFINES *= PORTABLE_BUILD }
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

!mac:unix {
    binary_folder = /usr/bin
    library_folder = /usr/lib
    arch_lib_path = /usr/lib/$${QT_ARCH}-linux-gnu
    exists($$arch_lib_path): library_folder = $$arch_lib_path
    # Define a reasonable default for share_folder
    share_folder = /usr/share

    !equals(d_prefix, "") {
        binary_folder = "$$d_prefix"/bin
        library_folder = "$$d_prefix"/lib
        share_folder = "$$d_prefix"/share
    }

    !equals(d_share, "") {
        share_folder = "$$d_share"
    }

    data_folder = $$share_folder/qupzilla
    launcher_folder = $$share_folder/applications
    icon_folder = $$share_folder/pixmaps
    hicolor_folder = $$share_folder/icons/hicolor

    !equals(d_use_lib_path, ""):library_folder = $$d_use_lib_path

    DEFINES *= USE_LIBPATH=\\\"""$$library_folder"\\\""
    DEFINES *= USE_DATADIR=\\\"""$$data_folder"\\\""

    # Define QZ_WS_X11 even with Qt5 (but only when building for X11)
    !contains(DEFINES, NO_X11) DEFINES *= QZ_WS_X11
}

unix: {
    # Git revision
    rev = $$system(cd ../ && sh $$PWD/../scripts/getrevision.sh)
    !equals(rev, ""): DEFINES *= GIT_REVISION=\\\"""$$rev"\\\""
}

isEmpty(QMAKE_LRELEASE) {
    win32|os2:QMAKE_LRELEASE = $$[QT_INSTALL_BINS]\\lrelease.exe
    else:QMAKE_LRELEASE = $$[QT_INSTALL_BINS]/lrelease

    # Try to use lrelease from PATH
    unix:!exists($$QMAKE_LRELEASE) {
        QMAKE_LRELEASE = lrelease
    }
}

isEmpty(QMAKE_LFLAGS_RPATH) {
    QMAKE_LFLAGS_RPATH = -Wl,-rpath,
}
