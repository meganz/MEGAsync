
set(DESKTOP_APP_STALLED_ISSUES_HEADERS
    ${CMAKE_CURRENT_LIST_DIR}/gui/StalledIssueChooseTitle.h
    ${CMAKE_CURRENT_LIST_DIR}/gui/StalledIssueActionTitle.h
    ${CMAKE_CURRENT_LIST_DIR}/gui/StalledIssueLoadingItem.h
    ${CMAKE_CURRENT_LIST_DIR}/gui/StalledIssueTab.h
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
    ${CMAKE_CURRENT_LIST_DIR}/gui/StalledIssueTab.cpp
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

target_sources_conditional(${ExecutableTarget}
   FLAG WIN32
   QT_AWARE
   PRIVATE
   ${CMAKE_CURRENT_LIST_DIR}/gui/win/StalledIssueHeader.ui
   ${CMAKE_CURRENT_LIST_DIR}/gui/win/StalledIssueChooseWidget.ui
   ${CMAKE_CURRENT_LIST_DIR}/gui/win/StalledIssuesDialog.ui
   ${CMAKE_CURRENT_LIST_DIR}/gui/win/StalledIssueFilePath.ui
   ${CMAKE_CURRENT_LIST_DIR}/gui/win/StalledIssueTab.ui
   ${CMAKE_CURRENT_LIST_DIR}/gui/win/StalledIssueLoadingItem.ui
   ${CMAKE_CURRENT_LIST_DIR}/gui/win/StalledIssueActionTitle.ui
   ${CMAKE_CURRENT_LIST_DIR}/gui/stalled_issues_cases/win/LocalAndRemoteDifferentWidget.ui
   ${CMAKE_CURRENT_LIST_DIR}/gui/stalled_issues_cases/win/FolderMatchedAgainstFileWidget.ui
   ${CMAKE_CURRENT_LIST_DIR}/gui/stalled_issues_cases/win/OtherSideMissingOrBlocked.ui
   ${CMAKE_CURRENT_LIST_DIR}/gui/stalled_issues_cases/win/NameConflict.ui
   ${CMAKE_CURRENT_LIST_DIR}/gui/stalled_issues_cases/win/LocalAndRemoteNameConflicts.ui
   ${CMAKE_CURRENT_LIST_DIR}/gui/stalled_issues_cases/win/MoveOrRenameCannotOccur.ui
)

target_sources_conditional(${ExecutableTarget}
   FLAG APPLE
   QT_AWARE
   PRIVATE
   ${CMAKE_CURRENT_LIST_DIR}/gui/macx/StalledIssueHeader.ui
   ${CMAKE_CURRENT_LIST_DIR}/gui/macx/StalledIssueChooseWidget.ui
   ${CMAKE_CURRENT_LIST_DIR}/gui/macx/StalledIssuesDialog.ui
   ${CMAKE_CURRENT_LIST_DIR}/gui/macx/StalledIssueFilePath.ui
   ${CMAKE_CURRENT_LIST_DIR}/gui/macx/StalledIssueTab.ui
   ${CMAKE_CURRENT_LIST_DIR}/gui/macx/StalledIssueLoadingItem.ui
   ${CMAKE_CURRENT_LIST_DIR}/gui/macx/StalledIssueActionTitle.ui
   ${CMAKE_CURRENT_LIST_DIR}/gui/stalled_issues_cases/macx/LocalAndRemoteDifferentWidget.ui
   ${CMAKE_CURRENT_LIST_DIR}/gui/stalled_issues_cases/macx/FolderMatchedAgainstFileWidget.ui
   ${CMAKE_CURRENT_LIST_DIR}/gui/stalled_issues_cases/macx/OtherSideMissingOrBlocked.ui
   ${CMAKE_CURRENT_LIST_DIR}/gui/stalled_issues_cases/macx/NameConflict.ui
   ${CMAKE_CURRENT_LIST_DIR}/gui/stalled_issues_cases/macx/LocalAndRemoteNameConflicts.ui
   ${CMAKE_CURRENT_LIST_DIR}/gui/stalled_issues_cases/macx/MoveOrRenameCannotOccur.ui
)

target_sources_conditional(${ExecutableTarget}
   FLAG UNIX AND NOT APPLE
   QT_AWARE
   PRIVATE
   ${CMAKE_CURRENT_LIST_DIR}/gui/linux/StalledIssueHeader.ui
   ${CMAKE_CURRENT_LIST_DIR}/gui/linux/StalledIssueChooseWidget.ui
   ${CMAKE_CURRENT_LIST_DIR}/gui/linux/StalledIssuesDialog.ui
   ${CMAKE_CURRENT_LIST_DIR}/gui/linux/StalledIssueFilePath.ui
   ${CMAKE_CURRENT_LIST_DIR}/gui/linux/StalledIssueTab.ui
   ${CMAKE_CURRENT_LIST_DIR}/gui/linux/StalledIssueLoadingItem.ui
   ${CMAKE_CURRENT_LIST_DIR}/gui/linux/StalledIssueActionTitle.ui
   ${CMAKE_CURRENT_LIST_DIR}/gui/stalled_issues_cases/linux/LocalAndRemoteDifferentWidget.ui
   ${CMAKE_CURRENT_LIST_DIR}/gui/stalled_issues_cases/linux/FolderMatchedAgainstFileWidget.ui
   ${CMAKE_CURRENT_LIST_DIR}/gui/stalled_issues_cases/linux/OtherSideMissingOrBlocked.ui
   ${CMAKE_CURRENT_LIST_DIR}/gui/stalled_issues_cases/linux/NameConflict.ui
   ${CMAKE_CURRENT_LIST_DIR}/gui/stalled_issues_cases/linux/LocalAndRemoteNameConflicts.ui
   ${CMAKE_CURRENT_LIST_DIR}/gui/stalled_issues_cases/linux/MoveOrRenameCannotOccur.ui
)


if (WIN32)
    set_property(TARGET ${ExecutableTarget}
        APPEND PROPERTY AUTOUIC_SEARCH_PATHS
        ${CMAKE_CURRENT_LIST_DIR}/gui/win
        ${CMAKE_CURRENT_LIST_DIR}/gui/stalled_issues_cases/win
    )
elseif (APPLE)
    set_property(TARGET ${ExecutableTarget}
        APPEND PROPERTY AUTOUIC_SEARCH_PATHS
        ${CMAKE_CURRENT_LIST_DIR}/gui/macx
        ${CMAKE_CURRENT_LIST_DIR}/gui/stalled_issues_cases/macx
    )
elseif (UNIX AND NOT APPLE)
    set_property(TARGET ${ExecutableTarget}
        APPEND PROPERTY AUTOUIC_SEARCH_PATHS
        ${CMAKE_CURRENT_LIST_DIR}/gui/linux
        ${CMAKE_CURRENT_LIST_DIR}/gui/stalled_issues_cases/linux
    )
endif()

target_sources(${ExecutableTarget}
    PRIVATE
    ${DESKTOP_APP_STALLED_ISSUES_HEADERS}
    ${DESKTOP_APP_STALLED_ISSUES_SOURCES}
)

set (INCLUDE_DIRECTORIES
    ${CMAKE_CURRENT_LIST_DIR}
    ${CMAKE_CURRENT_LIST_DIR}/gui
    ${CMAKE_CURRENT_LIST_DIR}/gui/stalled_issues_cases
    ${CMAKE_CURRENT_LIST_DIR}/model
)

target_include_directories(${ExecutableTarget} PRIVATE ${INCLUDE_DIRECTORIES})
