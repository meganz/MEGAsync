DEPENDPATH += $$PWD
INCLUDEPATH += $$PWD
INCLUDEPATH += $$PWD/gui
INCLUDEPATH += $$PWD/model

SOURCES +=  \
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
    $$PWD/gui/stalled_issues_cases/OtherSideMissingOrBlocked.cpp \
    $$PWD/gui/stalled_issues_cases/StalledIssuesCaseHeaders.cpp \
    $$PWD/model/StalledIssue.cpp \
    $$PWD/model/StalledIssuesModel.cpp \
    $$PWD/model/StalledIssuesProxyModel.cpp

HEADERS  +=   \
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
    $$PWD/gui/stalled_issues_cases/OtherSideMissingOrBlocked.h \
    $$PWD/gui/stalled_issues_cases/StalledIssuesCaseHeaders.h \
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
                $$PWD/gui/stalled_issues_cases/win/LocalAndRemoteDifferentWidget.ui \
                $$PWD/gui/stalled_issues_cases/win/OtherSideMissingOrBlocked.ui
}
