
set(DESKTOP_APP_STALLED_ISSUES_HEADERS
    stalled_issues/gui/StalledIssueChooseTitle.h
    stalled_issues/gui/StalledIssueActionTitle.h
    stalled_issues/gui/StalledIssueLoadingItem.h
    stalled_issues/gui/StalledIssueTab.h
    stalled_issues/gui/StalledIssueChooseWidget.h
    stalled_issues/gui/StalledIssueBaseDelegateWidget.h
    stalled_issues/gui/StalledIssueDelegate.h
    stalled_issues/gui/StalledIssueFilePath.h
    stalled_issues/gui/StalledIssuesView.h
    stalled_issues/gui/StalledIssuesDelegateWidgetsCache.h
    stalled_issues/gui/StalledIssuesDialog.h
    stalled_issues/gui/StalledIssueHeader.h
    stalled_issues/gui/stalled_issues_cases/LocalAndRemoteDifferentWidget.h
    stalled_issues/gui/stalled_issues_cases/LocalAndRemoteNameConflicts.h
    stalled_issues/gui/stalled_issues_cases/NameConflict.h
    stalled_issues/gui/stalled_issues_cases/OtherSideMissingOrBlocked.h
    stalled_issues/gui/stalled_issues_cases/StalledIssuesCaseHeaders.h
    stalled_issues/model/IgnoredStalledIssue.h
    stalled_issues/model/LocalOrRemoteUserMustChooseStalledIssue.h
    stalled_issues/model/MoveOrRenameCannotOccurIssue.h
    stalled_issues/model/NameConflictStalledIssue.h
    stalled_issues/model/StalledIssuesUtilities.h
    stalled_issues/model/StalledIssuesModel.h
    stalled_issues/model/StalledIssue.h
    stalled_issues/model/StalledIssuesProxyModel.h
)

set(DESKTOP_APP_STALLED_ISSUES_SOURCES
    stalled_issues/gui/StalledIssueChooseTitle.cpp
    stalled_issues/gui/StalledIssueActionTitle.cpp
    stalled_issues/gui/StalledIssueLoadingItem.cpp
    stalled_issues/gui/StalledIssueTab.cpp
    stalled_issues/gui/StalledIssueChooseWidget.cpp
    stalled_issues/gui/StalledIssueBaseDelegateWidget.cpp
    stalled_issues/gui/StalledIssueDelegate.cpp
    stalled_issues/gui/StalledIssueFilePath.cpp
    stalled_issues/gui/StalledIssuesView.cpp
    stalled_issues/gui/stalled_issues_cases/LocalAndRemoteDifferentWidget.cpp
    stalled_issues/gui/StalledIssuesDelegateWidgetsCache.cpp
    stalled_issues/gui/StalledIssuesDialog.cpp
    stalled_issues/gui/StalledIssueHeader.cpp
    stalled_issues/gui/stalled_issues_cases/LocalAndRemoteNameConflicts.cpp
    stalled_issues/gui/stalled_issues_cases/NameConflict.cpp
    stalled_issues/gui/stalled_issues_cases/OtherSideMissingOrBlocked.cpp
    stalled_issues/gui/stalled_issues_cases/StalledIssuesCaseHeaders.cpp
    stalled_issues/model/IgnoredStalledIssue.cpp
    stalled_issues/model/LocalOrRemoteUserMustChooseStalledIssue.cpp
    stalled_issues/model/MoveOrRenameCannotOccurIssue.cpp
    stalled_issues/model/NameConflictStalledIssue.cpp
    stalled_issues/model/StalledIssuesUtilities.cpp
    stalled_issues/model/StalledIssue.cpp
    stalled_issues/model/StalledIssuesModel.cpp
    stalled_issues/model/StalledIssuesProxyModel.cpp
)

target_sources_conditional(MEGAsync
   FLAG WIN32
   QT_AWARE
   PRIVATE
   stalled_issues/../gui/Resources_win.qrc
   stalled_issues/gui/win/StalledIssueHeader.ui
   stalled_issues/gui/win/StalledIssueChooseWidget.ui
   stalled_issues/gui/win/StalledIssuesDialog.ui
   stalled_issues/gui/win/StalledIssueFilePath.ui
   stalled_issues/gui/win/StalledIssueTab.ui
   stalled_issues/gui/win/StalledIssueLoadingItem.ui
   stalled_issues/gui/win/StalledIssueActionTitle.ui
   stalled_issues/gui/stalled_issues_cases/win/LocalAndRemoteDifferentWidget.ui
   stalled_issues/gui/stalled_issues_cases/win/OtherSideMissingOrBlocked.ui
   stalled_issues/gui/stalled_issues_cases/win/NameConflict.ui
   stalled_issues/gui/stalled_issues_cases/win/LocalAndRemoteNameConflicts.ui
)

target_sources_conditional(MEGAsync
   FLAG APPLE
   QT_AWARE
   PRIVATE
   stalled_issues/../gui/Resources_macx.qrc
   stalled_issues/gui/macx/StalledIssueHeader.ui
   stalled_issues/gui/macx/StalledIssueChooseWidget.ui
   stalled_issues/gui/macx/StalledIssuesDialog.ui
   stalled_issues/gui/macx/StalledIssueFilePath.ui
   stalled_issues/gui/macx/StalledIssueTab.ui
   stalled_issues/gui/macx/StalledIssueLoadingItem.ui
   stalled_issues/gui/macx/StalledIssueActionTitle.ui
   stalled_issues/gui/stalled_issues_cases/macx/LocalAndRemoteDifferentWidget.ui
   stalled_issues/gui/stalled_issues_cases/macx/OtherSideMissingOrBlocked.ui
   stalled_issues/gui/stalled_issues_cases/macx/NameConflict.ui
   stalled_issues/gui/stalled_issues_cases/macx/LocalAndRemoteNameConflicts.ui
)

if (WIN32)
    set_property(TARGET MEGAsync
        APPEND PROPERTY AUTOUIC_SEARCH_PATHS
        stalled_issues/gui/win
        stalled_issues/gui/stalled_issues_cases/win
    )
elseif (APPLE)
    set_property(TARGET MEGAsync
        APPEND PROPERTY AUTOUIC_SEARCH_PATHS
        stalled_issues/gui/macx
        stalled_issues/gui/stalled_issues_cases/macx
    )
endif()

target_sources(MEGAsync
    PRIVATE
    ${DESKTOP_APP_STALLED_ISSUES_HEADERS}
    ${DESKTOP_APP_STALLED_ISSUES_SOURCES}
)

target_include_directories(MEGAsync PRIVATE ${CMAKE_CURRENT_LIST_DIR}
    ${CMAKE_CURRENT_LIST_DIR}/gui
    ${CMAKE_CURRENT_LIST_DIR}/model
)
