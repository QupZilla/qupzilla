# Unix
!mac:unix {
    contains(DEFINES, USE_QTWEBKIT_2_2) {
        buildNotifications = true

        contains(DEFINES, USE_QTWEBKIT_2_3):system(pkg-config --exists hunspell) {
            buildSpellcheck = true
            LIBS += $$system(pkg-config --libs hunspell)
        }
    }
    else {
        buildPlugin = false
    }
}

# Mac OS X
mac {
    buildPlugin = false
}

# OS/2
os2 {
    buildPlugin = false
}

# Windows
win32 {
    win32-msvc* {
        # QtWebKit 2.3 and Hunspell is now needed to build on Windows
        buildNotifications = true
        buildSpellcheck = true
        LIBS += -llibhunspell
    }
    else { # mingw
        buildPlugin = false
    }
}

!equals(buildPlugin, false) {
    HEADERS += $$PWD/qtwebkitplugin.h \
               $$[QT_INSTALL_HEADERS]/QtWebKit/qwebkitplatformplugin.h

    SOURCES += $$PWD/qtwebkitplugin.cpp

    DEFINES *= QT_STATICPLUGIN
}
else {
    buildNotifications = false
    buildSpellcheck = false
}

equals(buildNotifications, true) {
    HEADERS += $$PWD/notifications/notificationpresenter.h
    SOURCES += $$PWD/notifications/notificationpresenter.cpp
}

equals(buildSpellcheck, true) {
    HEADERS += $$PWD/spellcheck/spellcheck.h \
        $$PWD/spellcheck/speller.h \
        $$PWD/spellcheck/spellcheckdialog.h \

    SOURCES += $$PWD/spellcheck/spellcheck.cpp \
        $$PWD/spellcheck/speller.cpp \
        $$PWD/spellcheck/spellcheckdialog.cpp \

    FORMS += $$PWD/spellcheck/spellcheckdialog.ui

    DEFINES *= USE_HUNSPELL
}
