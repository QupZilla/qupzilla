include(../../defines.pri)

TARGET = $$qtLibraryTarget(StatusBarIcons)

SOURCES += statusbariconsplugin.cpp \
    sbi_iconsmanager.cpp \
    sbi_imagesicon.cpp \
    sbi_javascripticon.cpp \
    sbi_networkicon.cpp \
    sbi_networkproxy.cpp \
    sbi_proxywidget.cpp \
    sbi_networkicondialog.cpp \
    sbi_networkmanager.cpp \
    sbi_settingsdialog.cpp

HEADERS += statusbariconsplugin.h \
    sbi_iconsmanager.h \
    sbi_imagesicon.h \
    sbi_javascripticon.h \
    sbi_networkicon.h \
    sbi_networkproxy.h \
    sbi_proxywidget.h \
    sbi_networkicondialog.h \
    sbi_networkmanager.h \
    sbi_settingsdialog.h

RESOURCES += statusbaricons.qrc

TRANSLATIONS += \
    translations/ar_SA.ts \
    translations/bg_BG.ts \
    translations/cs_CZ.ts \
    translations/de_DE.ts \
    translations/el_GR.ts \
    translations/es_ES.ts \
    translations/eu_ES.ts \
    translations/fr_FR.ts \
    translations/he_IL.ts \
    translations/it_IT.ts \
    translations/ja_JP.ts \
    translations/nl_NL.ts \
    translations/nqo.ts \
    translations/pl_PL.ts \
    translations/pt_PT.ts \
    translations/ru_RU.ts \
    translations/sr_BA@latin.ts \
    translations/sr_BA.ts \
    translations/sr_RS@latin.ts \
    translations/sr_RS.ts \
    translations/uk_UA.ts \
    translations/uz.ts \
    translations/zh_CN.ts \
    translations/zh_TW.ts \

include(../../plugins.pri)

FORMS += \
    sbi_proxywidget.ui \
    sbi_networkicondialog.ui \
    sbi_settingsdialog.ui
