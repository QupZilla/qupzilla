TARGET = $$qtLibraryTarget(StatusBarIcons)
os2: TARGET = StatusBa

SOURCES += statusbariconsplugin.cpp \
    sbi_iconsmanager.cpp \
    sbi_imagesicon.cpp \
    sbi_javascripticon.cpp \
    sbi_networkicon.cpp \
    sbi_networkproxy.cpp \
    sbi_proxywidget.cpp \
    sbi_networkicondialog.cpp \
    sbi_networkmanager.cpp \
    sbi_settingsdialog.cpp \
    sbi_icon.cpp \
    sbi_zoomwidget.cpp

HEADERS += statusbariconsplugin.h \
    sbi_iconsmanager.h \
    sbi_imagesicon.h \
    sbi_javascripticon.h \
    sbi_networkicon.h \
    sbi_networkproxy.h \
    sbi_proxywidget.h \
    sbi_networkicondialog.h \
    sbi_networkmanager.h \
    sbi_settingsdialog.h \
    sbi_icon.h \
    sbi_zoomwidget.h

RESOURCES += statusbaricons.qrc

TRANSLATIONS += \
    translations/ar_SA.ts \
    translations/bg_BG.ts \
    translations/ca_ES.ts \
    translations/cs_CZ.ts \
    translations/da_DK.ts \
    translations/de_DE.ts \
    translations/el_GR.ts \
    translations/es_ES.ts \
    translations/es_MX.ts \
    translations/es_VE.ts \
    translations/eu_ES.ts \
    translations/fa_IR.ts \
    translations/fi_FI.ts \
    translations/fr_FR.ts \
    translations/he_IL.ts \
    translations/hr_HR.ts \
    translations/id_ID.ts \
    translations/is.ts \
    translations/it_IT.ts \
    translations/ja_JP.ts \
    translations/lt.ts \
    translations/lv_LV.ts \
    translations/nb_NO.ts \
    translations/nl_NL.ts \
    translations/nqo.ts \
    translations/pl_PL.ts \
    translations/pt_PT.ts \
    translations/ru_RU.ts \
    translations/sk_SK.ts \
    translations/sl_SI.ts \
    translations/sr@ijekavianlatin.ts \
    translations/sr@ijekavian.ts \
    translations/sr@latin.ts \
    translations/sr.ts \
    translations/tr_TR.ts \
    translations/uk_UA.ts \
    translations/uz@Cyrl.ts \
    translations/uz@Latn.ts \
    translations/zh_CN.ts \
    translations/zh_HK.ts \
    translations/zh_TW.ts \

FORMS += \
    sbi_proxywidget.ui \
    sbi_networkicondialog.ui \
    sbi_settingsdialog.ui

PLUGIN_DIR = $$PWD
include(../../plugins.pri)
