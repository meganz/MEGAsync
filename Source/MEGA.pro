TEMPLATE = subdirs

SUBDIRS += MEGASync MEGAUpdater

win32 {
    SUBDIRS += MEGAShellExt MEGACrashAnalyzer
}

unix:!macx {
    SUBDIRS += MEGAShellExtNautilus
}
