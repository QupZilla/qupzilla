include(../../defines.pri)

TARGET = $$qtLibraryTarget(KWalletPasswords)

SOURCES += kwalletplugin.cpp \
    kwalletpasswordbackend.cpp

HEADERS += kwalletplugin.h \
    kwalletpasswordbackend.h

RESOURCES += kwalletpasswords.qrc

TRANSLATIONS += \
    translations/ar_SA.ts \
    translations/bg_BG.ts \
    translations/cs_CZ.ts \
    translations/de_DE.ts \
    translations/el_GR.ts \
    translations/es_AR.ts \
    translations/es_ES.ts \
    translations/es_MX.ts \
    translations/eu_ES.ts \
    translations/fa_IR.ts \
    translations/fr_FR.ts \
    translations/id_ID.ts \
    translations/it_IT.ts \
    translations/ja_JP.ts \
    translations/ko_KR.ts \
    translations/lv_LV.ts \
    translations/nb_NO.ts \
    translations/nl_NL.ts \
    translations/nqo.ts \
    translations/pl_PL.ts \
    translations/pt_PT.ts \
    translations/ru_RU.ts \
    translations/sk_SK.ts \
    translations/sr_BA@latin.ts \
    translations/sr_BA.ts \
    translations/sr_RS@latin.ts \
    translations/sr_RS.ts \
    translations/tr_TR.ts \
    translations/uk_UA.ts \
    translations/uz@Cyrl.ts \
    translations/uz@Latn.ts \
    translations/zh_CN.ts \
    translations/zh_TW.ts \

LIBS += -lkdeui

include(../../plugins.pri)
