DEPENDPATH += $$PWD
INCLUDEPATH += $$PWD
INCLUDEPATH += $$PWD/gui
INCLUDEPATH += $$PWD/model

SOURCES +=  \
    $$PWD/gui/StalledIssueChooseTitle.cpp \
    $$PWD/gui/StalledIssueLoadingItem.cpp \
    $$PWD/gui/StalledIssueTab.cpp \
    $$PWD/gui/StalledIssueChooseWidget.cpp \
    $$PWD/gui/StalledIssueBaseDelegateWidget.cpp \
    $$PWD/gui/StalledIssueDelegate.cpp \
    $$PWD/gui/StalledIssueFilePath.cpp \
    $$PWD/gui/StalledIssuesView.cpp \
    $$PWD/gui/stalled_issues_cases/LocalAndRemoteDifferentWidget.cpp \
    $$PWD/gui/StalledIssuesDelegateWidgetsCache.cpp \
    $$PWD/gui/StalledIssuesDialog.cpp \
    $$PWD/gui/StalledIssueHeader.cpp \
    $$PWD/gui/stalled_issues_cases/LocalAndRemoteNameConflicts.cpp \
    $$PWD/gui/stalled_issues_cases/NameConflict.cpp \
    $$PWD/gui/stalled_issues_cases/OtherSideMissingOrBlocked.cpp \
    $$PWD/gui/stalled_issues_cases/StalledIssuesCaseHeaders.cpp \
    $$PWD/model/StalledIssuesUtilities.cpp \
    $$PWD/model/StalledIssue.cpp \
    $$PWD/model/StalledIssuesModel.cpp \
    $$PWD/model/StalledIssuesProxyModel.cpp

HEADERS  +=   \
    $$PWD/gui/StalledIssueChooseTitle.h \
    $$PWD/gui/StalledIssueLoadingItem.h \
    $$PWD/gui/StalledIssueTab.h \
    $$PWD/gui/StalledIssueChooseWidget.h \
    $$PWD/gui/StalledIssueBaseDelegateWidget.h \
    $$PWD/gui/StalledIssueDelegate.h \
    $$PWD/gui/StalledIssueFilePath.h \
    $$PWD/gui/StalledIssuesView.h \
    $$PWD/gui/StalledIssuesDelegateWidgetsCache.h \
    $$PWD/gui/StalledIssuesDialog.h \
    $$PWD/gui/StalledIssueHeader.h \
    $$PWD/gui/stalled_issues_cases/LocalAndRemoteDifferentWidget.h \
    $$PWD/gui/stalled_issues_cases/LocalAndRemoteNameConflicts.h \
    $$PWD/gui/stalled_issues_cases/NameConflict.h \
    $$PWD/gui/stalled_issues_cases/OtherSideMissingOrBlocked.h \
    $$PWD/gui/stalled_issues_cases/StalledIssuesCaseHeaders.h \
    $$PWD/model/StalledIssuesUtilities.h \
    $$PWD/model/StalledIssuesModel.h \
    $$PWD/model/StalledIssue.h \
    $$PWD/model/StalledIssuesProxyModel.h

win32 {
    RESOURCES += $$PWD/../gui/Resources_win.qrc
    INCLUDEPATH += $$PWD/gui/win
    FORMS    += $$PWD/gui/win/StalledIssueHeader.ui \
                $$PWD/gui/win/StalledIssueChooseWidget.ui \
                $$PWD/gui/win/StalledIssuesDialog.ui \
                $$PWD/gui/win/StalledIssueFilePath.ui \
                $$PWD/gui/win/StalledIssueTab.ui \
                $$PWD/gui/win/StalledIssueLoadingItem.ui \
                $$PWD/gui/win/StalledIssueChooseTitle.ui \
                $$PWD/gui/stalled_issues_cases/win/LocalAndRemoteDifferentWidget.ui \
                $$PWD/gui/stalled_issues_cases/win/OtherSideMissingOrBlocked.ui \
                $$PWD/gui/stalled_issues_cases/win/NameConflict.ui \
                $$PWD/gui/stalled_issues_cases/win/LocalAndRemoteNameConflicts.ui
}

macx {
    RESOURCES += $$PWD/../gui/Resources_macx.qrc
    INCLUDEPATH += $$PWD/gui/macx
    FORMS    += $$PWD/gui/macx/StalledIssueHeader.ui \
                $$PWD/gui/macx/StalledIssueChooseWidget.ui \
                $$PWD/gui/macx/StalledIssuesDialog.ui \
                $$PWD/gui/macx/StalledIssueFilePath.ui \
                $$PWD/gui/macx/StalledIssueTab.ui \
                $$PWD/gui/macx/StalledIssueLoadingItem.ui \
                $$PWD/gui/macx/StalledIssueChooseTitle.ui \
                $$PWD/gui/stalled_issues_cases/macx/LocalAndRemoteDifferentWidget.ui \
                $$PWD/gui/stalled_issues_cases/macx/OtherSideMissingOrBlocked.ui \
                $$PWD/gui/stalled_issues_cases/macx/NameConflict.ui \
                $$PWD/gui/stalled_issues_cases/macx/LocalAndRemoteNameConflicts.ui
}

unix:!macx {
RESOURCES += $$PWD/../gui/Resources_linux.qrc
INCLUDEPATH += $$PWD/gui/linux
FORMS    += $$PWD/gui/linux/StalledIssueHeader.ui \
            $$PWD/gui/linux/StalledIssueChooseWidget.ui \
            $$PWD/gui/linux/StalledIssuesDialog.ui \
            $$PWD/gui/linux/StalledIssueFilePath.ui \
            $$PWD/gui/linux/StalledIssueTab.ui \
            $$PWD/gui/linux/StalledIssueLoadingItem.ui \
            $$PWD/gui/linux/StalledIssueChooseTitle.ui \
            $$PWD/gui/stalled_issues_cases/linux/LocalAndRemoteDifferentWidget.ui \
            $$PWD/gui/stalled_issues_cases/linux/OtherSideMissingOrBlocked.ui \
            $$PWD/gui/stalled_issues_cases/linux/NameConflict.ui \
            $$PWD/gui/stalled_issues_cases/linux/LocalAndRemoteNameConflicts.ui
}
