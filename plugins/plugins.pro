TEMPLATE = subdirs

sub_lib.subdir = libqupzilla
sub_testplugin.subdir = TestPlugin
sub_testplugin.depends = sub_lib

SUBDIRS  = sub_lib sub_testplugin
