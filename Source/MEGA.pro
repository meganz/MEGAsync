TEMPLATE = subdirs

SUBDIRS += MEGASync

win32 {
    SUBDIRS += MEGAShellExt MEGACrashAnalyzer MEGAUpdater
}

macx {
    SUBDIRS += MEGAUpdater
}

# qmake "CONFIG+=with_ext" MEGA.pro
unix:!macx {
    CONFIG(with_ext) {
        SUBDIRS += MEGAShellExtNautilus
    }
}
