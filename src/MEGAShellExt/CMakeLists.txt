#
# MEGAShellExt
# Library responsible for the integration with windows explorer
#

project(MEGAShellExt
    DESCRIPTION "MEGAShellExt"
    )

add_library(MEGAShellExt SHARED)

set(MEGAShellExt_HEADERS
    W11ShellContextMenu/data/ContextMenuData.h
    W11ShellContextMenu/gui/ContextMenuCommand.h
    W11ShellContextMenu/gui/ContextMenuCommandBase.h
    W11ShellContextMenu/gui/ContextMenuCommandGetLink.h
    W11ShellContextMenu/gui/ContextMenuCommandRemoveFromLeftPane.h
    W11ShellContextMenu/gui/ContextMenuCommandSeparator.h
    W11ShellContextMenu/gui/ContextMenuCommandUpload.h
    W11ShellContextMenu/gui/ContextMenuCommandView.h
    W11ShellContextMenu/gui/ContextMenuCommandViewVersions.h
    W11ShellContextMenu/gui/SubCommandEnumerator.h
    W11ShellContextMenu/utilities/framework.h
    W11ShellContextMenu/utilities/SparsePackageManager.h
    W11ShellContextMenu/utilities/SharedState.h
    W11ShellContextMenu/utilities/SimpleFactory.h
    W11ShellContextMenu/utilities/Utilities.h
    ShellExt.h
    ShellExtNotASync.h
    resource.h
    RegUtils.h
    LegacyShellContextMenu/ContextMenuExt.h
    ClassFactoryShellExtSyncing.h
    ClassFactoryShellExtSynced.h
    ClassFactoryShellExtPending.h
    ClassFactoryShellExtNotFound.h
    ClassFactoryContextMenuExt.h
    ClassFactory.h
    MegaInterface.h
)

set(MEGAShellExt_SOURCES
    W11ShellContextMenu/data/ContextMenuData.cpp
    W11ShellContextMenu/gui/ContextMenuCommandBase.cpp
    W11ShellContextMenu/gui/ContextMenuCommand.cpp
    W11ShellContextMenu/gui/ContextMenuCommandGetLink.cpp
    W11ShellContextMenu/gui/ContextMenuCommandRemoveFromLeftPane.cpp
    W11ShellContextMenu/gui/ContextMenuCommandSeparator.cpp
    W11ShellContextMenu/gui/ContextMenuCommandUpload.cpp
    W11ShellContextMenu/gui/ContextMenuCommandView.cpp
    W11ShellContextMenu/gui/ContextMenuCommandViewVersions.cpp
    W11ShellContextMenu/utilities/SparsePackageManager.cpp
    W11ShellContextMenu/utilities/SharedState.cpp
    W11ShellContextMenu/utilities/Utilities.cpp
    ShellExt.cpp
    ShellExtNotASync.cpp
    RegUtils.cpp
    dllmain.cpp
    LegacyShellContextMenu/ContextMenuExt.cpp
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
    ${CMAKE_CURRENT_SOURCE_DIR}/W11ShellContextMenu/data
    ${CMAKE_CURRENT_SOURCE_DIR}/W11ShellContextMenu/gui
    ${CMAKE_CURRENT_SOURCE_DIR}/W11ShellContextMenu/utilities
    ${CMAKE_CURRENT_SOURCE_DIR}/LegacyShellContextMenu
)

target_compile_features(MEGAShellExt PRIVATE cxx_std_17)

#if(ENABLE_EXPLORER_EXT_SPARSE)
    if(MSVC)
        # Create resources .pri
        add_custom_command(
            TARGET MEGAShellExt
            POST_BUILD
            COMMAND makepri createconfig "/o" "/cf" "${CMAKE_CURRENT_SOURCE_DIR}/packaging/priconfig.xml" "/dq" "en-US"
            COMMENT "Creating priconfig.xml"
        )

        add_custom_command(
            TARGET MEGAShellExt
            POST_BUILD
            COMMAND MakePri.exe new "/cf" "${CMAKE_CURRENT_SOURCE_DIR}/packaging/priconfig.xml" "/pr" "${CMAKE_CURRENT_SOURCE_DIR}"
            "/mn" "${CMAKE_CURRENT_SOURCE_DIR}/packaging/AppAplication.xml" "/o" "/of" "${CMAKE_CURRENT_SOURCE_DIR}/packaging/resources.pri"
            COMMENT "Creating resources.pri"
        )

        # Create sparse package
        add_custom_command(
            TARGET MEGAShellExt
            POST_BUILD
            COMMAND makeappx pack "/nv" "/o" "/d" "${CMAKE_CURRENT_SOURCE_DIR}/packaging" "/p" "$<TARGET_FILE_DIR:MEGAShellExt>/MEGAShellExt.msix"
            COMMENT "Creating sparse package for Win11 context menus"
        )

        # Sign sparse package
        add_custom_command(
            TARGET MEGAShellExt
            POST_BUILD
            COMMAND signtool sign "/n" "Mega Limited" "/td" SHA256 "/fd" SHA256 "$<TARGET_FILE_DIR:MEGAShellExt>/MEGAShellExt.msix"
            COMMENT "Signing sparse package"
        )

        # Sign dll
        add_custom_command(
            TARGET MEGAShellExt
            POST_BUILD
            COMMAND signtool sign "/n" "Mega Limited"  "/td" SHA256 "/fd" SHA256 $<TARGET_FILE:MEGAShellExt>
            COMMENT "Signing DLL"
        )
    endif()
#endif()
