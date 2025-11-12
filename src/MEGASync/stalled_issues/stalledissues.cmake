
set(DESKTOP_APP_STALLED_ISSUES_HEADERS
    ${CMAKE_CURRENT_LIST_DIR}/gui/StalledIssueChooseTitle.h
    ${CMAKE_CURRENT_LIST_DIR}/gui/StalledIssueActionTitle.h
    ${CMAKE_CURRENT_LIST_DIR}/gui/StalledIssueLoadingItem.h
    ${CMAKE_CURRENT_LIST_DIR}/gui/StalledIssueChooseWidget.h
    ${CMAKE_CURRENT_LIST_DIR}/gui/StalledIssueBaseDelegateWidget.h
    ${CMAKE_CURRENT_LIST_DIR}/gui/StalledIssueDelegate.h
    ${CMAKE_CURRENT_LIST_DIR}/gui/StalledIssueFilePath.h
    ${CMAKE_CURRENT_LIST_DIR}/gui/StalledIssuesView.h
    ${CMAKE_CURRENT_LIST_DIR}/gui/StalledIssuesDelegateWidgetsCache.h
    ${CMAKE_CURRENT_LIST_DIR}/gui/StalledIssuesDialog.h
    ${CMAKE_CURRENT_LIST_DIR}/gui/StalledIssueHeader.h
    ${CMAKE_CURRENT_LIST_DIR}/gui/stalled_issues_cases/LocalAndRemoteDifferentWidget.h
    ${CMAKE_CURRENT_LIST_DIR}/gui/stalled_issues_cases/FolderMatchedAgainstFileWidget.h
    ${CMAKE_CURRENT_LIST_DIR}/gui/stalled_issues_cases/LocalAndRemoteNameConflicts.h
    ${CMAKE_CURRENT_LIST_DIR}/gui/stalled_issues_cases/NameConflict.h
    ${CMAKE_CURRENT_LIST_DIR}/gui/stalled_issues_cases/OtherSideMissingOrBlocked.h
    ${CMAKE_CURRENT_LIST_DIR}/gui/stalled_issues_cases/StalledIssuesCaseHeaders.h
    ${CMAKE_CURRENT_LIST_DIR}/gui/stalled_issues_cases/MoveOrRenameCannotOccur.h
    ${CMAKE_CURRENT_LIST_DIR}/gui/stalled_issues_cases/LocalAndRemoteChooseWidget.h
    ${CMAKE_CURRENT_LIST_DIR}/gui/stalled_issues_cases/MoveOrRenameCannotOccurChooseWidget.h
    ${CMAKE_CURRENT_LIST_DIR}/model/IgnoredStalledIssue.h
    ${CMAKE_CURRENT_LIST_DIR}/model/DownloadFileIssue.h
    ${CMAKE_CURRENT_LIST_DIR}/model/LocalOrRemoteUserMustChooseStalledIssue.h
    ${CMAKE_CURRENT_LIST_DIR}/model/MoveOrRenameCannotOccurIssue.h
    ${CMAKE_CURRENT_LIST_DIR}/model/NameConflictStalledIssue.h
    ${CMAKE_CURRENT_LIST_DIR}/model/FolderMatchedAgainstFileIssue.h
    ${CMAKE_CURRENT_LIST_DIR}/model/StalledIssuesUtilities.h
    ${CMAKE_CURRENT_LIST_DIR}/model/StalledIssuesModel.h
    ${CMAKE_CURRENT_LIST_DIR}/model/StalledIssue.h
    ${CMAKE_CURRENT_LIST_DIR}/model/StalledIssuesProxyModel.h
    ${CMAKE_CURRENT_LIST_DIR}/model/StalledIssuesFactory.h
    ${CMAKE_CURRENT_LIST_DIR}/model/MultiStepIssueSolver.h
)

set(DESKTOP_APP_STALLED_ISSUES_SOURCES
    ${CMAKE_CURRENT_LIST_DIR}/gui/StalledIssueChooseTitle.cpp
    ${CMAKE_CURRENT_LIST_DIR}/gui/StalledIssueActionTitle.cpp
    ${CMAKE_CURRENT_LIST_DIR}/gui/StalledIssueLoadingItem.cpp
    ${CMAKE_CURRENT_LIST_DIR}/gui/StalledIssueChooseWidget.cpp
    ${CMAKE_CURRENT_LIST_DIR}/gui/StalledIssueBaseDelegateWidget.cpp
    ${CMAKE_CURRENT_LIST_DIR}/gui/StalledIssueDelegate.cpp
    ${CMAKE_CURRENT_LIST_DIR}/gui/StalledIssueFilePath.cpp
    ${CMAKE_CURRENT_LIST_DIR}/gui/StalledIssuesView.cpp
    ${CMAKE_CURRENT_LIST_DIR}/gui/StalledIssuesDelegateWidgetsCache.cpp
    ${CMAKE_CURRENT_LIST_DIR}/gui/StalledIssuesDialog.cpp
    ${CMAKE_CURRENT_LIST_DIR}/gui/StalledIssueHeader.cpp
    ${CMAKE_CURRENT_LIST_DIR}/gui/stalled_issues_cases/LocalAndRemoteDifferentWidget.cpp
    ${CMAKE_CURRENT_LIST_DIR}/gui/stalled_issues_cases/FolderMatchedAgainstFileWidget.cpp
    ${CMAKE_CURRENT_LIST_DIR}/gui/stalled_issues_cases/LocalAndRemoteNameConflicts.cpp
    ${CMAKE_CURRENT_LIST_DIR}/gui/stalled_issues_cases/NameConflict.cpp
    ${CMAKE_CURRENT_LIST_DIR}/gui/stalled_issues_cases/OtherSideMissingOrBlocked.cpp
    ${CMAKE_CURRENT_LIST_DIR}/gui/stalled_issues_cases/StalledIssuesCaseHeaders.cpp
    ${CMAKE_CURRENT_LIST_DIR}/gui/stalled_issues_cases/MoveOrRenameCannotOccur.cpp
    ${CMAKE_CURRENT_LIST_DIR}/gui/stalled_issues_cases/LocalAndRemoteChooseWidget.cpp
    ${CMAKE_CURRENT_LIST_DIR}/gui/stalled_issues_cases/MoveOrRenameCannotOccurChooseWidget.cpp
    ${CMAKE_CURRENT_LIST_DIR}/model/IgnoredStalledIssue.cpp
    ${CMAKE_CURRENT_LIST_DIR}/model/DownloadFileIssue.cpp
    ${CMAKE_CURRENT_LIST_DIR}/model/LocalOrRemoteUserMustChooseStalledIssue.cpp
    ${CMAKE_CURRENT_LIST_DIR}/model/MoveOrRenameCannotOccurIssue.cpp
    ${CMAKE_CURRENT_LIST_DIR}/model/NameConflictStalledIssue.cpp
    ${CMAKE_CURRENT_LIST_DIR}/model/FolderMatchedAgainstFileIssue.cpp
    ${CMAKE_CURRENT_LIST_DIR}/model/StalledIssuesUtilities.cpp
    ${CMAKE_CURRENT_LIST_DIR}/model/StalledIssue.cpp
    ${CMAKE_CURRENT_LIST_DIR}/model/StalledIssuesModel.cpp
    ${CMAKE_CURRENT_LIST_DIR}/model/StalledIssuesProxyModel.cpp
    ${CMAKE_CURRENT_LIST_DIR}/model/StalledIssuesFactory.cpp
    ${CMAKE_CURRENT_LIST_DIR}/model/MultiStepIssueSolver.cpp
)

set(DESKTOP_APP_STALLED_ISSUES_UI_FILES
    ${CMAKE_CURRENT_LIST_DIR}/gui/ui/StalledIssueHeader.ui
    ${CMAKE_CURRENT_LIST_DIR}/gui/ui/StalledIssueChooseWidget.ui
    ${CMAKE_CURRENT_LIST_DIR}/gui/ui/StalledIssuesDialog.ui
    ${CMAKE_CURRENT_LIST_DIR}/gui/ui/StalledIssueFilePath.ui
    ${CMAKE_CURRENT_LIST_DIR}/gui/ui/StalledIssueLoadingItem.ui
    ${CMAKE_CURRENT_LIST_DIR}/gui/ui/StalledIssueActionTitle.ui
    ${CMAKE_CURRENT_LIST_DIR}/gui/stalled_issues_cases/ui/LocalAndRemoteDifferentWidget.ui
    ${CMAKE_CURRENT_LIST_DIR}/gui/stalled_issues_cases/ui/FolderMatchedAgainstFileWidget.ui
    ${CMAKE_CURRENT_LIST_DIR}/gui/stalled_issues_cases/ui/OtherSideMissingOrBlocked.ui
    ${CMAKE_CURRENT_LIST_DIR}/gui/stalled_issues_cases/ui/NameConflict.ui
    ${CMAKE_CURRENT_LIST_DIR}/gui/stalled_issues_cases/ui/LocalAndRemoteNameConflicts.ui
    ${CMAKE_CURRENT_LIST_DIR}/gui/stalled_issues_cases/ui/MoveOrRenameCannotOccur.ui
)

set_property(TARGET ${ExecutableTarget}
    APPEND PROPERTY AUTOUIC_SEARCH_PATHS
    ${CMAKE_CURRENT_LIST_DIR}/gui/ui
    ${CMAKE_CURRENT_LIST_DIR}/gui/stalled_issues_cases/ui
)

target_sources(${ExecutableTarget}
    PRIVATE
    ${DESKTOP_APP_STALLED_ISSUES_HEADERS}
    ${DESKTOP_APP_STALLED_ISSUES_SOURCES}
    ${DESKTOP_APP_STALLED_ISSUES_UI_FILES}
)

set (INCLUDE_DIRECTORIES
    ${CMAKE_CURRENT_LIST_DIR}
    ${CMAKE_CURRENT_LIST_DIR}/gui
    ${CMAKE_CURRENT_LIST_DIR}/gui/stalled_issues_cases
    ${CMAKE_CURRENT_LIST_DIR}/model
)

target_include_directories(${ExecutableTarget} PRIVATE ${INCLUDE_DIRECTORIES})
