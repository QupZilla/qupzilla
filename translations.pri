TRANSLATIONS += $$PWD/translations/cs_CZ.ts\
                $$PWD/translations/sk_SK.ts\
                $$PWD/translations/de_DE.ts\
                $$PWD/translations/nl_NL.ts\
                $$PWD/translations/zh_CN.ts\
                $$PWD/translations/zh_TW.ts\
                $$PWD/translations/it_IT.ts\
                $$PWD/translations/pl_PL.ts\
                $$PWD/translations/es_ES.ts\
                $$PWD/translations/fr_FR.ts\
                $$PWD/translations/el_GR.ts\
                $$PWD/translations/ru_RU.ts\
                $$PWD/translations/pt_PT.ts\
                $$PWD/translations/sr_BA.ts\
                $$PWD/translations/sr_RS.ts\
                $$PWD/translations/sv_SE.ts\
                $$PWD/translations/id_ID.ts\

isEmpty(QMAKE_LRELEASE) {
    win32|os2:QMAKE_LRELEASE = $$[QT_INSTALL_BINS]\\lrelease.exe
    else:QMAKE_LRELEASE = $$[QT_INSTALL_BINS]/lrelease
    unix {
        !exists($$QMAKE_LRELEASE) { QMAKE_LRELEASE = lrelease-qt4 }
    } else {
        !exists($$QMAKE_LRELEASE) { QMAKE_LRELEASE = lrelease }
    }
}

updateqm.input = TRANSLATIONS
updateqm.output = $$PWD/bin/locale/${QMAKE_FILE_BASE}.qm
updateqm.commands = $$QMAKE_LRELEASE -silent ${QMAKE_FILE_IN} -qm $$PWD/bin/locale/${QMAKE_FILE_BASE}.qm
updateqm.CONFIG += no_link target_predeps
QMAKE_EXTRA_COMPILERS += updateqm
