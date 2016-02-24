TEMPLATE = subdirs

SUBDIRS += MEGASync

CONFIG(debug, debug|release) {
    #SUBDIRS += MEGALogger
}

win32 {
    #SUBDIRS += MEGACrashAnalyzer
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

macx {
    SUBDIRS += MEGALoader
}

CONFIG(with_updater) {
    SUBDIRS += MEGAUpdater
}

CONFIG(with_tools) {
    SUBDIRS += MEGASync/mega/contrib/QtCreator/MEGACli
    SUBDIRS += MEGASync/mega/contrib/QtCreator/MEGASimplesync
}
