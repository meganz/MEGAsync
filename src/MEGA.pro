TEMPLATE = subdirs

SUBDIRS += MEGASync

CONFIG(debug, debug|release) {
    #SUBDIRS += MEGALogger
}

win32 {
    #SUBDIRS += MEGAUpdater
    SUBDIRS += MEGACrashAnalyzer
    CONFIG(with_ext) {
        SUBDIRS += MEGAShellExt
    }
}

# qmake "CONFIG+=with_ext" MEGA.pro
unix:!macx {
    CONFIG(with_ext) {
        SUBDIRS += MEGAShellExtNautilus
        SUBDIRS += MEGAShellExtThunar
        SUBDIRS += MEGAShellExtDolphin
        SUBDIRS += MEGAShellExtNemo
    }
}

macx {
    SUBDIRS += MEGAUpdater
}

CONFIG(with_updater) {
    SUBDIRS += MEGAUpdateGenerator
}

CONFIG(with_tests) {
    SUBDIRS += ../tests/MEGASyncUnitTests
}

CONFIG(with_tools) {
    SUBDIRS += MEGASync/mega/contrib/QtCreator/MEGACli
    SUBDIRS += MEGASync/mega/contrib/QtCreator/MEGASimplesync
}
