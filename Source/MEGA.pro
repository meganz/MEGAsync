TEMPLATE = subdirs

SUBDIRS += MEGASync MEGAUpdater MEGACrashAnalyzer

win32 {
    SUBDIRS += MEGAShellExt
}

unix:!macx {
    SUBDIRS += MEGAShellExtNautilus
}
