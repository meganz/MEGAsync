TEMPLATE = subdirs

SUBDIRS += MEGASync

win32 {
    SUBDIRS += MEGACrashAnalyzer
    CONFIG(with_ext) {
	SUBDIRS += MEGAShellExt
    }
}

# qmake "CONFIG+=with_ext" MEGA.pro
unix:!macx {
    CONFIG(with_ext) {
        SUBDIRS += MEGAShellExtNautilus
    }
}

CONFIG(with_updater) {
    SUBDIRS += MEGAUpdater
}
