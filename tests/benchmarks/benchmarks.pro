TEMPLATE = subdirs

include(benchmarks.pri)

defineTest(addSubdir) {
    for(subdir, 1) {
        entries = $$files($$subdir/*)
        for(entry, entries) {
            fullPath = $$replace(entry, ;,"")
            fullPath = $$replace(fullPath, \\\\, /)
            name = $$replace(fullPath, $$re_escape("$$subdir/"), "")
            os2|win32: fullPath = $$lower($$fullPath)
            exists($$fullPath/*.pro): SUBDIRS += $$fullPath
        }
    }

    export (SUBDIRS)
}

addSubdir($$PWD)