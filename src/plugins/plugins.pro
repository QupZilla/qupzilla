TEMPLATE = subdirs

include(../defines.pri)

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

defineTest(disablePlugin) {
    SUBDIRS -= $$PWD/$$1
    os2|win32: SUBDIRS -= $$lower($$PWD/$$1)

    export(SUBDIRS)
}

addSubdir($$PWD)

outOfDirPlugins = $$(QUPZILLA_PLUGINS_SRCDIR)
!equals(outOfDirPlugins, ""): addSubdir($$(QUPZILLA_PLUGINS_SRCDIR))

# TestPlugin only in debug build
!CONFIG(debug, debug|release): disablePlugin(TestPlugin)

# KWalletPasswords only with KDE_INTEGRATION
!contains(DEFINES, KDE_INTEGRATION) | !qtHaveModule(KWallet): disablePlugin(KWalletPasswords)

# GnomeKeyringPasswords only with GNOME_INTEGRATION
!contains(DEFINES, GNOME_INTEGRATION) | !system(pkg-config --exists gnome-keyring-1): disablePlugin(GnomeKeyringPasswords)
