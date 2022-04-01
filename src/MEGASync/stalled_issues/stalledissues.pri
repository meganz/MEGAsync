DEPENDPATH += $$PWD
INCLUDEPATH += $$PWD
INCLUDEPATH += $$PWD/gui
INCLUDEPATH += $$PWD/model

SOURCES +=  \
    $$PWD/gui/StalledIssueBaseDelegateWidget.cpp \
    $$PWD/gui/StalledIssueDelegate.cpp \
    $$PWD/model/StalledIssue.cpp \
    $$PWD/model/StalledIssuesModel.cpp \
    $$PWD/gui/StalledIssuesDialog.cpp \
    $$PWD/gui/StalledIssueHeader.cpp \
    $$PWD/model/StalledIssuesProxyModel.cpp

HEADERS  +=   \
    $$PWD/gui/StalledIssueBaseDelegateWidget.h \
    $$PWD/gui/StalledIssueDelegate.h \
    $$PWD/model/StalledIssue.h \
    $$PWD/model/StalledIssuesModel.h \
    $$PWD/gui/StalledIssuesDialog.h \
    $$PWD/gui/StalledIssueHeader.h \
    $$PWD/model/StalledIssuesProxyModel.h

win32 {
    RESOURCES += $$PWD/../gui/Resources_win.qrc
    INCLUDEPATH += $$PWD/gui/win
    FORMS    += $$PWD/gui/win/StalledIssueHeader.ui \
                $$PWD/gui/win/StalledIssuesDialog.ui
}
