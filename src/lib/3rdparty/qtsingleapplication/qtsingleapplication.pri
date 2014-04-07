DEPENDPATH += $$PWD
SOURCES += $$PWD/qtsingleapplication.cpp $$PWD/qtlocalpeer.cpp
HEADERS += $$PWD/qtsingleapplication.h $$PWD/qtlocalpeer.h

os2|win32 {
    contains(TEMPLATE, lib):contains(CONFIG, shared):DEFINES *= QT_QTSINGLEAPPLICATION_EXPORT
    else:qtsingleapplication-uselib:DEFINES *= QT_QTSINGLEAPPLICATION_IMPORT
}
