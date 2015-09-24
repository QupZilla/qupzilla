TARGET = $$qtLibraryTarget(FlashCookieManager)
os2: TARGET = FlashCoo

SOURCES += fcm_plugin.cpp \
    fcm_dialog.cpp \
    fcm_notification.cpp

HEADERS += fcm_plugin.h \
    fcm_dialog.h \
    fcm_notification.h

RESOURCES += flashcookiemanager.qrc

TRANSLATIONS += \
    translations/bg_BG.ts \
    translations/cs_CZ.ts \
    translations/de_DE.ts \
    translations/el_GR.ts \
    translations/es_ES.ts \
    translations/es_MX.ts \
    translations/eu_ES.ts \
    translations/fa_IR.ts \
    translations/fr_FR.ts \
    translations/he_IL.ts \
    translations/hr_HR.ts \
    translations/id_ID.ts \
    translations/it_IT.ts \
    translations/lt.ts \
    translations/nl_NL.ts \
    translations/pl_PL.ts \
    translations/pt_PT.ts \
    translations/ru_RU.ts \
    translations/sr@ijekavianlatin.ts \
    translations/sr@ijekavian.ts \
    translations/sr@latin.ts \
    translations/sr.ts \
    translations/tr_TR.ts \
    translations/zh_CN.ts \
    translations/zh_TW.ts \

PLUGIN_DIR = $$PWD
srcdir = $$(QUPZILLA_SRCDIR)
equals(srcdir, "") {
    include(../../plugins.pri)
}
else {
    include($$srcdir/src/plugins.pri)
}

FORMS += \
    fcm_dialog.ui \
    fcm_notification.ui
