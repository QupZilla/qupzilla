TEMPLATE = subdirs

include(../defines.pri)

defineTest(addSubdir) {
    for(subdir, 1) {
        entries = $$files($$subdir/*)
        for(entry, entries) {
            fullPath = $$replace(entry, ;,"")
            fullPath = $$replace(fullPath, \\\\, /)
            name = $$replace(fullPath, $$re_escape("$$subdir/"), "")
            win32: fullPath = $$lower($$fullPath)
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

# KWalletPasswords only with KDE_INTEGRATION
!contains(DEFINES, KDE_INTEGRATION): SUBDIRS -= $$PWD/KWalletPasswords
!lessThan(QT_VERSION, 5.0): SUBDIRS -= $$PWD/KWalletPasswords

# GnomeKeyringPasswords only with GNOME_INTEGRATION
!contains(DEFINES, GNOME_INTEGRATION): SUBDIRS -= $$PWD/GnomeKeyringPasswords
!system(pkg-config --exists gnome-keyring-1): SUBDIRS -= $$PWD/GnomeKeyringPasswords

!unix|mac {
    SUBDIRS -= $$lower($$PWD/KWalletPasswords)
    SUBDIRS -= $$lower($$PWD/GnomeKeyringPasswords)
}
