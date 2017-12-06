include(defines.pri)

mac {
    QMAKE_INFO_PLIST = $$PWD/main/Info.plist
    ICON = $$PWD/lib/data/icons/exeicons/qupzilla.icns

    bundle_target.files += $$QZ_DESTDIR/locale
    bundle_target.files += $$QZ_DESTDIR/themes
    bundle_target.files += $$QZ_DESTDIR/plugins
    bundle_target.path = Contents/Resources

    QMAKE_BUNDLE_DATA += bundle_target
}

!mac:unix {
    target.path = $$binary_folder

    target1.files += $$QZ_DESTDIR/locale
    target1.files += $$QZ_DESTDIR/themes
    target1.path = $$data_folder

    target2.files = $$PWD/../linux/applications/org.qupzilla.QupZilla.desktop
    target2.path = $$launcher_folder

    target3.files = $$PWD/../linux/pixmaps/qupzilla.png
    target3.path = $$icon_folder

    ico16.files = $$PWD/../linux/hicolor/16x16/apps/qupzilla.png
    ico16.path = $$hicolor_folder/16x16/apps

    ico32.files = $$PWD/../linux/hicolor/32x32/apps/qupzilla.png
    ico32.path = $$hicolor_folder/32x32/apps

    ico48.files = $$PWD/../linux/hicolor/48x48/apps/qupzilla.png
    ico48.path = $$hicolor_folder/48x48/apps

    ico64.files = $$PWD/../linux/hicolor/64x64/apps/qupzilla.png
    ico64.path = $$hicolor_folder/64x64/apps

    ico128.files = $$PWD/../linux/hicolor/128x128/apps/qupzilla.png
    ico128.path = $$hicolor_folder/128x128/apps

    ico256.files = $$PWD/../linux/hicolor/256x256/apps/qupzilla.png
    ico256.path = $$hicolor_folder/256x256/apps

    bashcompletion.files = $$PWD/../linux/completion/qupzilla
    bashcompletion.path = $$share_folder/bash-completion/completions

    appdata.files = $$PWD/../linux/appdata/org.qupzilla.QupZilla.appdata.xml
    appdata.path = $$share_folder/metainfo


    INSTALLS += target target1 target2 target3
    INSTALLS += ico16 ico32 ico48 ico64 ico128 ico256
    INSTALLS += bashcompletion
    INSTALLS += appdata
}
