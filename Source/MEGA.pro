TEMPLATE = subdirs

SUBDIRS += MEGASync MEGAUpdater

win32 {
    SUBDIRS += MEGAShellExt MEGACrashAnalyzer
}

# qmake "CONFIG+=with_ext" MEGA.pro
unix:!macx {
    CONFIG(with_ext) {
        SUBDIRS += MEGAShellExtNautilus
    }
}
