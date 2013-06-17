TRANSLATIONS += $$PWD/ca_ES.ts \
                $$PWD/cs_CZ.ts \
                $$PWD/de_DE.ts \
                $$PWD/el_GR.ts \
                $$PWD/es_ES.ts \
                $$PWD/es_VE.ts \
                $$PWD/fa_IR.ts \
                $$PWD/fr_FR.ts \
                $$PWD/he_IL.ts \
                $$PWD/hu_HU.ts \
                $$PWD/id_ID.ts \
                $$PWD/it_IT.ts \
                $$PWD/ja_JP.ts \
                $$PWD/ka_GE.ts \
                $$PWD/nl_NL.ts \
                $$PWD/pl_PL.ts \
                $$PWD/pt_BR.ts \
                $$PWD/pt_PT.ts \
                $$PWD/ro_RO.ts \
                $$PWD/ru_RU.ts \
                $$PWD/sk_SK.ts \
                $$PWD/sr_BA.ts \
                $$PWD/sr_BA@latin.ts \
                $$PWD/sr_RS.ts \
                $$PWD/sr_RS@latin.ts \
                $$PWD/sv_SE.ts \
                $$PWD/uk_UA.ts \
                $$PWD/zh_CN.ts \
                $$PWD/zh_TW.ts \

updateqm.input = TRANSLATIONS
updateqm.output = $$PWD/../bin/locale/${QMAKE_FILE_BASE}.qm
updateqm.commands = $$QMAKE_LRELEASE -silent ${QMAKE_FILE_IN} -qm $$PWD/../bin/locale/${QMAKE_FILE_BASE}.qm
updateqm.CONFIG += no_link target_predeps
QMAKE_EXTRA_COMPILERS += updateqm
