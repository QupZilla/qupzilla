TEMPLATE = subdirs

defineTest(addSubdir) {
    for(subdir, 1) {
        entries = $$files($$subdir/*)
        for(entry, entries) {
            fullPath = $$replace(entry, ;,"")
            name = $$replace(fullPath, $$re_escape("$$subdir/"), "")
            exists($$fullPath/*.pro): SUBDIRS += $$fullPath
        }
    }

    export (SUBDIRS)
}

addSubdir($$PWD)

outOfDirPlugins = $$(QUPZILLA_PLUGINS_SRCDIR)
!equals(outOfDirPlugins, "") : addSubdir($$(QUPZILLA_PLUGINS_SRCDIR))

# TestPlugin only in debug build
!CONFIG(debug, debug|release): SUBDIRS -= $$PWD/TestPlugin
