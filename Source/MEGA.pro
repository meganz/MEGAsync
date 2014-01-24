TEMPLATE = subdirs

SUBDIRS += MEGASync MEGAUpdater

win32 {
    SUBDIRS += MEGAShellExt
}

unix:!macx {
    SUBDIRS += MEGAShellExtNautilus
}
