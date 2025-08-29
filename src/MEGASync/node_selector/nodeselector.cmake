set(DESKTOP_APP_NODE_SELECTOR_HEADERS
    ${CMAKE_CURRENT_LIST_DIR}/model/NodeSelectorDelegates.h
    ${CMAKE_CURRENT_LIST_DIR}/model/NodeSelectorProxyModel.h
    ${CMAKE_CURRENT_LIST_DIR}/model/NodeSelectorModel.h
    ${CMAKE_CURRENT_LIST_DIR}/model/NodeSelectorModelSpecialised.h
    ${CMAKE_CURRENT_LIST_DIR}/model/NodeSelectorModelItem.h
    ${CMAKE_CURRENT_LIST_DIR}/model/RestoreNodeManager.h
    ${CMAKE_CURRENT_LIST_DIR}/gui/NodeSelectorTreeView.h
    ${CMAKE_CURRENT_LIST_DIR}/gui/NodeSelectorTreeViewWidget.h
    ${CMAKE_CURRENT_LIST_DIR}/gui/NodeSelectorTreeViewWidgetSpecializations.h
    ${CMAKE_CURRENT_LIST_DIR}/gui/NodeSelector.h
    ${CMAKE_CURRENT_LIST_DIR}/gui/NodeSelectorLoadingDelegate.h
    ${CMAKE_CURRENT_LIST_DIR}/gui/SearchLineEdit.h
    ${CMAKE_CURRENT_LIST_DIR}/gui/NodeSelectorSpecializations.h
)

set(DESKTOP_APP_NODE_SELECTOR_SOURCES
    ${CMAKE_CURRENT_LIST_DIR}/model/NodeSelectorDelegates.cpp
    ${CMAKE_CURRENT_LIST_DIR}/model/NodeSelectorProxyModel.cpp
    ${CMAKE_CURRENT_LIST_DIR}/model/NodeSelectorModel.cpp
    ${CMAKE_CURRENT_LIST_DIR}/model/NodeSelectorModelSpecialised.cpp
    ${CMAKE_CURRENT_LIST_DIR}/model/NodeSelectorModelItem.cpp
    ${CMAKE_CURRENT_LIST_DIR}/model/RestoreNodeManager.cpp
    ${CMAKE_CURRENT_LIST_DIR}/gui/NodeSelectorTreeView.cpp
    ${CMAKE_CURRENT_LIST_DIR}/gui/NodeSelectorTreeViewWidget.cpp
    ${CMAKE_CURRENT_LIST_DIR}/gui/NodeSelectorTreeViewWidgetSpecializations.cpp
    ${CMAKE_CURRENT_LIST_DIR}/gui/NodeSelector.cpp
    ${CMAKE_CURRENT_LIST_DIR}/gui/NodeSelectorLoadingDelegate.cpp
    ${CMAKE_CURRENT_LIST_DIR}/gui/SearchLineEdit.cpp
    ${CMAKE_CURRENT_LIST_DIR}/gui/NodeSelectorSpecializations.cpp
)

set(DESKTOP_APP_NODE_SELECTOR_UI_FILES
    ${CMAKE_CURRENT_LIST_DIR}/gui/ui/NodeSelectorTreeViewWidget.ui
    ${CMAKE_CURRENT_LIST_DIR}/gui/ui/NodeSelectorLoadingDelegate.ui
    ${CMAKE_CURRENT_LIST_DIR}/gui/ui/NodeSelector.ui
    ${CMAKE_CURRENT_LIST_DIR}/gui/ui/SearchLineEdit.ui
)

set_property(TARGET ${ExecutableTarget}
    APPEND PROPERTY AUTOUIC_SEARCH_PATHS
    ${CMAKE_CURRENT_LIST_DIR}/gui/ui
)

target_sources(${ExecutableTarget}
    PRIVATE
    ${DESKTOP_APP_NODE_SELECTOR_HEADERS}
    ${DESKTOP_APP_NODE_SELECTOR_SOURCES}
    ${DESKTOP_APP_NODE_SELECTOR_UI_FILES}
)

set (INCLUDE_DIRECTORIES
    ${CMAKE_CURRENT_LIST_DIR}
    ${CMAKE_CURRENT_LIST_DIR}/gui
    ${CMAKE_CURRENT_LIST_DIR}/model
)

target_include_directories(${ExecutableTarget} PRIVATE ${INCLUDE_DIRECTORIES})
