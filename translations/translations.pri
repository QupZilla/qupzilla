TRANSLATIONS += \
    $$PWD/ar_SA.ts \
    $$PWD/bg_BG.ts \
    $$PWD/ca_ES.ts \
    $$PWD/cs_CZ.ts \
    $$PWD/da_DK.ts \
    $$PWD/de_DE.ts \
    $$PWD/el_GR.ts \
    $$PWD/es_ES.ts \
    $$PWD/es_MX.ts \
    $$PWD/es_VE.ts \
    $$PWD/eu_ES.ts \
    $$PWD/fa_IR.ts \
    $$PWD/fi_FI.ts \
    $$PWD/fr_FR.ts \
    $$PWD/gl_ES.ts \
    $$PWD/he_IL.ts \
    $$PWD/hr_HR.ts \
    $$PWD/hu_HU.ts \
    $$PWD/id_ID.ts \
    $$PWD/it_IT.ts \
    $$PWD/ja_JP.ts \
    $$PWD/ka_GE.ts \
    $$PWD/lg.ts \
    $$PWD/lt.ts \
    $$PWD/lv_LV.ts \
    $$PWD/nl_NL.ts \
    $$PWD/nqo.ts \
    $$PWD/pl_PL.ts \
    $$PWD/pt_BR.ts \
    $$PWD/pt_PT.ts \
    $$PWD/ro_RO.ts \
    $$PWD/ru_RU.ts \
    $$PWD/sk_SK.ts \
    $$PWD/sr@ijekavianlatin.ts \
    $$PWD/sr@ijekavian.ts \
    $$PWD/sr@latin.ts \
    $$PWD/sr.ts \
    $$PWD/sv_SE.ts \
    $$PWD/tr_TR.ts \
    $$PWD/uk_UA.ts \
    $$PWD/uz@Latn.ts \
    $$PWD/zh_CN.ts \
    $$PWD/zh_TW.ts \

updateqm.input = TRANSLATIONS
updateqm.output = $$PWD/../bin/locale/${QMAKE_FILE_BASE}.qm
updateqm.commands = $$QMAKE_LRELEASE -silent ${QMAKE_FILE_IN} -qm $$PWD/../bin/locale/${QMAKE_FILE_BASE}.qm
updateqm.CONFIG += no_link target_predeps
QMAKE_EXTRA_COMPILERS += updateqm
