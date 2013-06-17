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
    translations/ca_ES.ts \
    translations/cs_CZ.ts \
    translations/da_DK.ts \
    translations/de_DE.ts \
    translations/el_GR.ts \
    translations/es_ES.ts \
    translations/es_VE.ts \
    translations/fa_IR.ts \
    translations/fr_FR.ts \
    translations/gl_ES.ts \
    translations/he_IL.ts \
    translations/hu_HU.ts \
    translations/id_ID.ts \
    translations/it_IT.ts \
    translations/ja_JP.ts \
    translations/ka_GE.ts \
    translations/nl_NL.ts \
    translations/pl_PL.ts \
    translations/pt_BR.ts \
    translations/pt_PT.ts \
    translations/ro_RO.ts \
    translations/ru_RU.ts \
    translations/sk_SK.ts \
    translations/sr_BA.ts \
    translations/sr_BA@latin.ts \
    translations/sr_RS.ts \
    translations/sr_RS@latin.ts \
    translations/sv_SE.ts \
    translations/tr_TR.ts \
    translations/uk_UA.ts \
    translations/zh_CN.ts \
    translations/zh_TW.ts \

srcdir = $$(QUPZILLA_SRCDIR)
equals(srcdir, "") {
    include(../../plugins.pri)
}
else {
    include($$srcdir/src/plugins.pri)
}
