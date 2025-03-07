#
# MEGAShellExt
# Library responsible for the integration with windows explorer
#

project(MEGAShellExt
    DESCRIPTION "MEGAShellExt"
    )

add_library(MEGAShellExt SHARED)

set(MEGAShellExt_HEADERS
    ModernShellContextMenu/data/ContextMenuData.h
    ModernShellContextMenu/gui/ContextMenuCommand.h
    ModernShellContextMenu/gui/ContextMenuCommandBase.h
    ModernShellContextMenu/gui/ContextMenuCommandGetLink.h
    ModernShellContextMenu/gui/ContextMenuCommandRemoveFromLeftPane.h
    ModernShellContextMenu/gui/ContextMenuCommandSeparator.h
    ModernShellContextMenu/gui/ContextMenuCommandUpload.h
    ModernShellContextMenu/gui/ContextMenuCommandView.h
    ModernShellContextMenu/gui/ContextMenuCommandViewVersions.h
    ModernShellContextMenu/gui/SubCommandEnumerator.h
    ModernShellContextMenu/utilities/framework.h
    ModernShellContextMenu/utilities/SparsePackageManager.h
    ModernShellContextMenu/utilities/SimpleFactory.h
    ModernShellContextMenu/utilities/Utilities.h
    ShellExt.h
    ShellExtNotASync.h
    resource.h
    RegUtils.h
    ClassicShellContextMenu/ContextMenuExt.h
    ClassFactoryShellExtSyncing.h
    ClassFactoryShellExtSynced.h
    ClassFactoryShellExtPending.h
    ClassFactoryShellExtNotFound.h
    ClassFactoryContextMenuExt.h
    ClassFactory.h
    MegaInterface.h
)

set(MEGAShellExt_SOURCES
    ModernShellContextMenu/data/ContextMenuData.cpp
    ModernShellContextMenu/gui/ContextMenuCommandBase.cpp
    ModernShellContextMenu/gui/ContextMenuCommand.cpp
    ModernShellContextMenu/gui/ContextMenuCommandGetLink.cpp
    ModernShellContextMenu/gui/ContextMenuCommandRemoveFromLeftPane.cpp
    ModernShellContextMenu/gui/ContextMenuCommandSeparator.cpp
    ModernShellContextMenu/gui/ContextMenuCommandUpload.cpp
    ModernShellContextMenu/gui/ContextMenuCommandView.cpp
    ModernShellContextMenu/gui/ContextMenuCommandViewVersions.cpp
    ModernShellContextMenu/utilities/SparsePackageManager.cpp
    ModernShellContextMenu/utilities/Utilities.cpp
    ShellExt.cpp
    ShellExtNotASync.cpp
    RegUtils.cpp
    dllmain.cpp
    ClassicShellContextMenu/ContextMenuExt.cpp
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
    ${CMAKE_CURRENT_SOURCE_DIR}/ModernShellContextMenu/data
    ${CMAKE_CURRENT_SOURCE_DIR}/ModernShellContextMenu/gui
    ${CMAKE_CURRENT_SOURCE_DIR}/ModernShellContextMenu/utilities
    ${CMAKE_CURRENT_SOURCE_DIR}/ClassicShellContextMenu
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
