TRANSLATIONS += $$PWD/cs_CZ.ts\
                $$PWD/sk_SK.ts\
                $$PWD/de_DE.ts\
                $$PWD/nl_NL.ts\
                $$PWD/zh_CN.ts\
                $$PWD/zh_TW.ts\
                $$PWD/it_IT.ts\
                $$PWD/pl_PL.ts\
                $$PWD/es_ES.ts\
                $$PWD/es_VE.ts\
                $$PWD/fr_FR.ts\
                $$PWD/el_GR.ts\
                $$PWD/ru_RU.ts\
                $$PWD/pt_PT.ts\
                $$PWD/pt_BR.ts\
                $$PWD/sr_BA.ts\
                $$PWD/sr_BA@latin.ts\
                $$PWD/sr_RS.ts\
                $$PWD/sr_RS@latin.ts\
                $$PWD/sv_SE.ts\
                $$PWD/id_ID.ts\
                $$PWD/ka_GE.ts\
                $$PWD/ja_JP.ts\
                $$PWD/ro_RO.ts\
                $$PWD/hu_HU.ts\
                $$PWD/uk_UA.ts\
                $$PWD/fa_IR.ts\

updateqm.input = TRANSLATIONS
updateqm.output = $$PWD/../bin/locale/${QMAKE_FILE_BASE}.qm
updateqm.commands = $$QMAKE_LRELEASE -silent ${QMAKE_FILE_IN} -qm $$PWD/../bin/locale/${QMAKE_FILE_BASE}.qm
updateqm.CONFIG += no_link target_predeps
QMAKE_EXTRA_COMPILERS += updateqm
