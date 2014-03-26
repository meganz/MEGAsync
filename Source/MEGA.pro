TEMPLATE = subdirs

SUBDIRS += MEGASync

win32 {
    SUBDIRS += MEGAShellExt MEGAUpdater MEGACrashAnalyzer
}

unix:!macx {
    SUBDIRS += MEGAShellExtNautilus
}
