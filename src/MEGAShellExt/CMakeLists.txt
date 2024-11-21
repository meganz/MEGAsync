#
# MEGAShellExt
# Library responsible for the integration with windows explorer
#

project(MEGAShellExt
    DESCRIPTION "MEGAShellExt"
    )

add_library(MEGAShellExt SHARED)

set(MEGAShellExt_HEADERS
    newContextMenu/data/ContextMenuData.h
    newContextMenu/gui/ContextMenuCommandBase.h
    newContextMenu/gui/ContextMenuCommandGetLink.h
    newContextMenu/gui/ContextMenuCommandRemoveFromLeftPane.h
    newContextMenu/gui/ContextMenuCommandSeparator.h
    newContextMenu/gui/ContextMenuCommandUpload.h
    newContextMenu/gui/ContextMenuCommandView.h
    newContextMenu/gui/ContextMenuCommandViewVersions.h
    newContextMenu/utilities/framework.h
    newContextMenu/utilities/SparsePackageManager.h
    newContextMenu/utilities/SharedState.h
    newContextMenu/utilities/SimpleFactory.h
    newContextMenu/utilities/Utilities.h
    ShellExt.h
    ShellExtNotASync.h
    resource.h
    RegUtils.h
    oldContextMenu/ContextMenuExt.h
    ClassFactoryShellExtSyncing.h
    ClassFactoryShellExtSynced.h
    ClassFactoryShellExtPending.h
    ClassFactoryShellExtNotFound.h
    ClassFactoryContextMenuExt.h
    ClassFactory.h
    MegaInterface.h
)

set(MEGAShellExt_SOURCES
    newContextMenu/data/ContextMenuData.cpp
    newContextMenu/gui/ContextMenuCommandBase.cpp
    newContextMenu/gui/ContextMenuCommandGetLink.cpp
    newContextMenu/gui/ContextMenuCommandRemoveFromLeftPane.cpp
    newContextMenu/gui/ContextMenuCommandSeparator.cpp
    newContextMenu/gui/ContextMenuCommandUpload.cpp
    newContextMenu/gui/ContextMenuCommandView.cpp
    newContextMenu/gui/ContextMenuCommandViewVersions.cpp
    newContextMenu/utilities/SparsePackageManager.cpp
    newContextMenu/utilities/SharedState.cpp
    newContextMenu/utilities/Utilities.cpp
    ShellExt.cpp
    ShellExtNotASync.cpp
    RegUtils.cpp
    dllmain.cpp
    oldContextMenu/ContextMenuExt.cpp
    ClassFactoryShellExtSyncing.cpp
    ClassFactoryShellExtSynced.cpp
    ClassFactoryShellExtPending.cpp
    ClassFactoryShellExtNotFound.cpp
    ClassFactoryContextMenuExt.cpp
    ClassFactory.cpp
    MegaInterface.cpp
)

set(MEGAShellExt_RESOURCES
    ${CMAKE_CURRENT_SOURCE_DIR}/MEGAShellExt.rc
    ${CMAKE_CURRENT_SOURCE_DIR}/GlobalExportFunctions.def
)

target_sources(MEGAShellExt
    PRIVATE
    ${MEGAShellExt_HEADERS}
    ${MEGAShellExt_SOURCES}
    ${MEGAShellExt_RESOURCES}
)

target_compile_definitions(MEGAShellExt
    PUBLIC
    UNICODE
    _UNICODE
)

target_link_libraries(MEGAShellExt
    PRIVATE
    user32 ole32 oleaut32 gdi32 uuid Advapi32 Shell32
)

target_include_directories(MEGAShellExt
    PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}/newContextMenu/data
    ${CMAKE_CURRENT_SOURCE_DIR}/newContextMenu/gui
    ${CMAKE_CURRENT_SOURCE_DIR}/newContextMenu/utilities
    ${CMAKE_CURRENT_SOURCE_DIR}/oldContextMenu
)

target_compile_features(MEGAShellExt PRIVATE cxx_std_17)

#if(ENABLE_EXPLORER_EXT_SPARSE)
    if(MSVC)
        # Create sparse package
        add_custom_command(
            TARGET MEGAShellExt
            POST_BUILD
            COMMAND makeappx pack "/nv" "/o" "/d" "${CMAKE_CURRENT_SOURCE_DIR}/newContextMenu/gui/packaging" "/p" "MEGAShellExt.msix"
            COMMENT "Creating sparse package for Win11 context menus"
        )

        # Sign sparse package
        add_custom_command(
            TARGET MEGAShellExt
            POST_BUILD
            COMMAND signtool sign "/sha1" $ENV{SHELL_EXT_CERT_SHA} "/td" SHA256 "/fd" SHA256 "MEGAShellExt.msix"
            COMMENT "Signing sparse package"
        )

        # Sign dll
        add_custom_command(
            TARGET MEGAShellExt
            POST_BUILD
            COMMAND signtool sign "/sha1" $ENV{SHELL_EXT_CERT_SHA} "/td" SHA256 "/fd" SHA256 "MEGAShellExt.dll"
            COMMENT "Signing DLL"
        )
    endif()
#endif()
