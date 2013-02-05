HEADERS += $$PWD/qtwebkitplugin.h \
           $$PWD/notifications/notificationpresenter.h \
           $$[QT_INSTALL_HEADERS]/QtWebKit/qwebkitplatformplugin.h \


SOURCES += $$PWD/qtwebkitplugin.cpp \
           $$PWD/notifications/notificationpresenter.cpp \

DEFINES *= QT_STATICPLUGIN

unix:contains(DEFINES, USE_QTWEBKIT_2_3):system(pkg-config --exists hunspell) {
    HEADERS += $$PWD/spellcheck/spellcheck.h \
        $$PWD/spellcheck/speller.h \
        $$PWD/spellcheck/spellcheckdialog.h \

    SOURCES += $$PWD/spellcheck/spellcheck.cpp \
        $$PWD/spellcheck/speller.cpp \
        $$PWD/spellcheck/spellcheckdialog.cpp \

    FORMS += $$PWD/spellcheck/spellcheckdialog.ui

    DEFINES *= USE_HUNSPELL
    LIBS += $$system(pkg-config --libs hunspell)
}

win{
# TODO
}
