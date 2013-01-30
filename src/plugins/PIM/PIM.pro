TARGET = $$qtLibraryTarget(PIM)
os2: TARGET  = PIM

SOURCES = \
    PIM_plugin.cpp \
    PIM_handler.cpp \
    PIM_settings.cpp

HEADERS = \
    PIM_plugin.h \
    PIM_handler.h \
    PIM_settings.h

FORMS += \
    PIM_settings.ui

RESOURCES = PIM_res.qrc

TRANSLATIONS = \
    translations/cs_CZ.ts \
    translations/de_DE.ts \
    translations/es_ES.ts \
    translations/es_VE.ts \
    translations/fr_FR.ts \
    translations/id_ID.ts \
    translations/it_IT.ts \
    translations/pt_BR.ts \
    translations/pt_PT.ts \
    translations/sr_BA.ts \
    translations/sr_BA@latin.ts \
    translations/sr_RS.ts \
    translations/sr_RS@latin.ts \
    translations/uk_UA.ts \
    translations/zh_TW.ts \
    translations/fa_IR.ts \

srcdir = $$(QUPZILLA_SRCDIR)
equals(srcdir, "") {
    include(../../plugins.pri)
}
else {
    include($$srcdir/src/plugins.pri)
}
