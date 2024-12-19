
set(DESKTOP_APP_TRANSFERS_HEADERS
    transfers/model/InfoDialogTransfersProxyModel.h
    transfers/gui/DuplicatedNodeDialogs/DuplicatedNodeDialog.h
    transfers/gui/DuplicatedNodeDialogs/DuplicatedNodeInfo.h
    transfers/gui/DuplicatedNodeDialogs/DuplicatedNodeItem.h
    transfers/gui/DuplicatedNodeDialogs/DuplicatedUploadChecker.h
    transfers/gui/InfoDialogTransferLoadingItem.h
    transfers/model/TransfersManagerSortFilterProxyModel.h
    transfers/model/TransfersSortFilterProxyBaseModel.h
    transfers/model/TransfersModel.h
    transfers/model/TransferMetaData.h
    transfers/gui/SomeIssuesOccurredMessage.h
    transfers/gui/InfoDialogTransferDelegateWidget.h
    transfers/gui/InfoDialogTransfersWidget.h
    transfers/gui/MegaTransferDelegate.h
    transfers/gui/MegaTransferView.h
    transfers/gui/MediaTypeFilterWidget.h
    transfers/gui/TransferBaseDelegateWidget.h
    transfers/gui/TransferItem.h
    transfers/gui/TransferManager.h
    transfers/gui/TransferManagerDelegateWidget.h
    transfers/gui/TransferManagerLoadingItem.h
    transfers/gui/TransfersStatusWidget.h
    transfers/gui/TransfersSummaryWidget.h
    transfers/gui/TransferScanCancelUi.h
    transfers/gui/TransferWidgetHeaderItem.h
    transfers/gui/TransfersWidget.h
    transfers/gui/TransfersAccountInfoWidget.h
    transfers/gui/TransferManagerStatusHeaderWidget.h
)

set(DESKTOP_APP_TRANSFERS_SOURCES
    transfers/model/TransfersModel.cpp
    transfers/gui/DuplicatedNodeDialogs/DuplicatedNodeDialog.cpp
    transfers/gui/DuplicatedNodeDialogs/DuplicatedNodeInfo.cpp
    transfers/gui/DuplicatedNodeDialogs/DuplicatedNodeItem.cpp
    transfers/gui/DuplicatedNodeDialogs/DuplicatedUploadChecker.cpp
    transfers/gui/InfoDialogTransferLoadingItem.cpp
    transfers/model/InfoDialogTransfersProxyModel.cpp
    transfers/model/TransfersManagerSortFilterProxyModel.cpp
    transfers/gui/SomeIssuesOccurredMessage.cpp
    transfers/model/TransferMetaData.cpp
    transfers/gui/InfoDialogTransferDelegateWidget.cpp
    transfers/gui/InfoDialogTransfersWidget.cpp
    transfers/gui/MegaTransferDelegate.cpp
    transfers/gui/MegaTransferView.cpp
    transfers/gui/MediaTypeFilterWidget.cpp
    transfers/gui/TransferBaseDelegateWidget.cpp
    transfers/gui/TransferItem.cpp
    transfers/gui/TransferManager.cpp
    transfers/gui/TransferManagerDelegateWidget.cpp
    transfers/gui/TransferManagerLoadingItem.cpp
    transfers/gui/TransfersStatusWidget.cpp
    transfers/gui/TransfersSummaryWidget.cpp
    transfers/gui/TransferScanCancelUi.cpp
    transfers/gui/TransferWidgetHeaderItem.cpp
    transfers/gui/TransfersWidget.cpp
    transfers/gui/TransfersAccountInfoWidget.cpp
    transfers/gui/TransferManagerStatusHeaderWidget.cpp
)


target_sources_conditional(MEGAsync
   FLAG WIN32
   QT_AWARE
   PRIVATE
   transfers/gui/win/TransferWidgetHeaderItem.ui
   transfers/gui/win/TransferManagerDelegateWidget.ui
   transfers/gui/win/TransferManager.ui
   transfers/gui/win/TransfersWidget.ui
   transfers/gui/win/TransferManagerLoadingItem.ui
   transfers/gui/win/TransferManagerDragBackDrop.ui
   transfers/gui/win/InfoDialogTransfersWidget.ui
   transfers/gui/win/InfoDialogTransferDelegateWidget.ui
   transfers/gui/win/InfoDialogTransferLoadingItem.ui
   transfers/gui/win/TransfersStatusWidget.ui
   transfers/gui/win/TransfersSummaryWidget.ui
   transfers/gui/win/SomeIssuesOccurredMessage.ui
   transfers/gui/win/TransfersAccountInfoWidget.ui
   transfers/gui/win/MediaTypeFilterWidget.ui
   transfers/gui/DuplicatedNodeDialogs/win/DuplicatedNodeDialog.ui
   transfers/gui/DuplicatedNodeDialogs/win/DuplicatedNodeItem.ui
)

target_sources_conditional(MEGAsync
   FLAG APPLE
   QT_AWARE
   PRIVATE
   transfers/gui/macx/TransferWidgetHeaderItem.ui
   transfers/gui/macx/TransferManagerDelegateWidget.ui
   transfers/gui/macx/TransferManager.ui
   transfers/gui/macx/TransfersWidget.ui
   transfers/gui/macx/TransferManagerLoadingItem.ui
   transfers/gui/macx/TransferManagerDragBackDrop.ui
   transfers/gui/macx/InfoDialogTransfersWidget.ui
   transfers/gui/macx/InfoDialogTransferDelegateWidget.ui
   transfers/gui/macx/InfoDialogTransferLoadingItem.ui
   transfers/gui/macx/TransfersStatusWidget.ui
   transfers/gui/macx/TransfersSummaryWidget.ui
   transfers/gui/macx/SomeIssuesOccurredMessage.ui
   transfers/gui/macx/TransfersAccountInfoWidget.ui
   transfers/gui/macx/MediaTypeFilterWidget.ui
   transfers/gui/DuplicatedNodeDialogs/macx/DuplicatedNodeDialog.ui
   transfers/gui/DuplicatedNodeDialogs/macx/DuplicatedNodeItem.ui
)

target_sources_conditional(MEGAsync
   FLAG UNIX AND NOT APPLE
   QT_AWARE
   PRIVATE
   transfers/gui/linux/TransferWidgetHeaderItem.ui
   transfers/gui/linux/TransferManagerDelegateWidget.ui
   transfers/gui/linux/TransferManager.ui
   transfers/gui/linux/TransfersWidget.ui
   transfers/gui/linux/TransferManagerLoadingItem.ui
   transfers/gui/linux/TransferManagerDragBackDrop.ui
   transfers/gui/linux/InfoDialogTransfersWidget.ui
   transfers/gui/linux/InfoDialogTransferDelegateWidget.ui
   transfers/gui/linux/InfoDialogTransferLoadingItem.ui
   transfers/gui/linux/TransfersStatusWidget.ui
   transfers/gui/linux/TransfersSummaryWidget.ui
   transfers/gui/linux/SomeIssuesOccurredMessage.ui
   transfers/gui/linux/TransfersAccountInfoWidget.ui
   transfers/gui/linux/MediaTypeFilterWidget.ui
   transfers/gui/DuplicatedNodeDialogs/linux/DuplicatedNodeDialog.ui
   transfers/gui/DuplicatedNodeDialogs/linux/DuplicatedNodeItem.ui
)

if (WIN32)
    set_property(TARGET MEGAsync
        APPEND PROPERTY AUTOUIC_SEARCH_PATHS
        transfers/gui/win
        transfers/gui/DuplicatedNodeDialogs/win
        transfers/gui/ui
    )
elseif (APPLE)
    set_property(TARGET MEGAsync
        APPEND PROPERTY AUTOUIC_SEARCH_PATHS
        transfers/gui/macx
        transfers/gui/DuplicatedNodeDialogs/macx
        transfers/gui/ui
    )
else()
    set_property(TARGET MEGAsync
        APPEND PROPERTY AUTOUIC_SEARCH_PATHS
        transfers/gui/linux
        transfers/gui/DuplicatedNodeDialogs/linux
        transfers/gui/ui
    )
endif()

set (DESKTOP_APP_TRANSFERS_UI_FILES
    ${CMAKE_CURRENT_LIST_DIR}/gui/ui/TransferManagerStatusHeaderWidget.ui
)

target_sources(MEGAsync
    PRIVATE
    ${DESKTOP_APP_TRANSFERS_HEADERS}
    ${DESKTOP_APP_TRANSFERS_SOURCES}
    ${DESKTOP_APP_TRANSFERS_UI_FILES}
)

set (INCLUDE_DIRECTORIES
    ${CMAKE_CURRENT_LIST_DIR}
    ${CMAKE_CURRENT_LIST_DIR}/model
    ${CMAKE_CURRENT_LIST_DIR}/gui
    ${CMAKE_CURRENT_LIST_DIR}/gui/ui
    ${CMAKE_CURRENT_LIST_DIR}/gui/DuplicatedNodeDialogs
)

target_include_directories(MEGAsync PRIVATE ${INCLUDE_DIRECTORIES})

