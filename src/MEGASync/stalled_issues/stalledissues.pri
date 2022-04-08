DEPENDPATH += $$PWD
INCLUDEPATH += $$PWD
INCLUDEPATH += $$PWD/gui
INCLUDEPATH += $$PWD/model

SOURCES +=  \
    $$PWD/gui/StalledIssueChooseWidget.cpp \
    $$PWD/gui/StalledIssueBaseDelegateWidget.cpp \
    $$PWD/gui/StalledIssueDelegate.cpp \
    $$PWD/gui/StalledIssueFilePath.cpp \
    $$PWD/gui/StalledIssuesView.cpp \
    $$PWD/gui/stalled_issues_cases/LocalAndRemotePreviouslyUnsynceDifferWidget.cpp \
    $$PWD/gui/StalledIssuesDelegateWidgetsCache.cpp \
    $$PWD/gui/StalledIssuesDialog.cpp \
    $$PWD/gui/StalledIssueHeader.cpp \
    $$PWD/gui/stalled_issues_cases/LocalAndRemotePreviouslyUnsynceDifferWidget.cpp \
    $$PWD/model/StalledIssue.cpp \
    $$PWD/model/StalledIssuesModel.cpp \
    $$PWD/model/StalledIssuesProxyModel.cpp

HEADERS  +=   \
    $$PWD/gui/StalledIssueChooseWidget.h \
    $$PWD/gui/StalledIssueBaseDelegateWidget.h \
    $$PWD/gui/StalledIssueDelegate.h \
    $$PWD/gui/StalledIssueFilePath.h \
    $$PWD/gui/StalledIssuesView.h \
    $$PWD/gui/stalled_issues_cases/LocalAndRemotePreviouslyUnsynceDifferWidget.h \
    $$PWD/gui/StalledIssuesDelegateWidgetsCache.h \
    $$PWD/gui/StalledIssuesDialog.h \
    $$PWD/gui/StalledIssueHeader.h \
    $$PWD/gui/stalled_issues_cases/LocalAndRemotePreviouslyUnsynceDifferWidget.h \
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
                $$PWD/gui/stalled_issues_cases/win/LocalAndRemotePreviouslyUnsynceDifferWidget.ui
}
