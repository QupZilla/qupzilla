DESTDIR = $$PWD/../bin
OBJECTS_DIR = $$PWD/../build
MOC_DIR = $$PWD/../build
RCC_DIR = $$PWD/../build
UI_DIR = $$PWD/../build
unix: VERSION = 1.3.1

# Please read BUILD information #
#DEFINES *= NO_SYSTEM_DATAPATH
#DEFINES *= USE_WEBGL
#DEFINES *= KDE
#DEFINES *= PORTABLE_BUILD
win32-msvc* {
    DEFINES *= W7API
    LIBS += User32.lib Ole32.lib Shell32.lib ShlWapi.lib Gdi32.lib ComCtl32.lib
}

DEFINES *= QT_NO_URL_CAST_FROM_STRING
DEFINES *= QT_USE_QSTRINGBUILDER

CONFIG(debug, debug|release): DEFINES *= QUPZILLA_DEBUG_BUILD

d_no_system_datapath = $$(NO_SYSTEM_DATAPATH)
d_use_webgl = $$(USE_WEBGL)
d_w7api = $$(W7API)
d_kde = $$(KDE)
d_portable = $$(PORTABLE_BUILD)
d_nonblock_dialogs = $$(NONBLOCK_JS_DIALOGS)
d_use_qtwebkit_2_2 = $$(USE_QTWEBKIT_2_2)
d_use_lib_path = $$(USE_LIBPATH)
d_disable_dbus = $$(DISABLE_DBUS)

equals(d_no_system_datapath, "true") { DEFINES *= NO_SYSTEM_DATAPATH }
equals(d_use_webgl, "true") { DEFINES *= USE_WEBGL }
win32-msvc* {
    equals(d_w7api, "true") { DEFINES *= W7API }
}
equals(d_kde, "true") { DEFINES *= KDE }
equals(d_portable, "true") { DEFINES *= PORTABLE_BUILD }
equals(d_nonblock_dialogs, "true") { DEFINES *= NONBLOCK_JS_DIALOGS }
equals(d_use_qtwebkit_2_2, "true") { DEFINES *= USE_QTWEBKIT_2_2 }
equals(d_disable_dbus, "true") { DEFINES *= DISABLE_DBUS }

!mac:unix {
    d_prefix = $$(QUPZILLA_PREFIX)
    binary_folder = /usr/bin
    library_folder = /usr/lib
    data_folder = /usr/share/qupzilla
    launcher_folder = /usr/share/applications
    icon_folder = /usr/share/pixmaps
    hicolor_folder = /usr/share/icons/hicolor

    !equals(d_prefix, "") {
        binary_folder = "$$d_prefix"bin
        library_folder = "$$d_prefix"lib
        data_folder = "$$d_prefix"share/qupzilla
        launcher_folder = "$$d_prefix"share/applications
        icon_folder = "$$d_prefix"share/pixmaps
        hicolor_folder = "$$d_prefix"share/icons/hicolor
    }

    !equals(d_use_lib_path, "") {
        library_folder = $$d_use_lib_path
        DEFINES *= USE_LIBPATH=\\\"""$$d_use_lib_path/"\\\""
    }

    DEFINES *= USE_DATADIR=\\\"""$$data_folder/"\\\""

    #Git revision
    rev = $$system(cd ../ && sh $$PWD/../scripts/getrevision.sh)
    !equals(rev, ""): DEFINES *= GIT_REVISION=\\\"""$$rev"\\\""
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
