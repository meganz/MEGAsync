#
# MEGAShellExt
# Library responsible for the integration with windows explorer
#

project(MEGAShellExt
    DESCRIPTION "MEGAShellExt"
    )

add_library(MEGAShellExt SHARED)

set(MEGAShellExt_HEADERS
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
    ClassicShellContextMenu/ContextMenuExt.h
    ContextMenuData.h
    ShellExt.h
    ShellExtNotASync.h
    resource.h
    RegUtils.h
    ClassFactoryShellExtSyncing.h
    ClassFactoryShellExtSynced.h
    ClassFactoryShellExtPending.h
    ClassFactoryShellExtNotFound.h
    ClassFactoryContextMenuExt.h
    ClassFactory.h
    MegaInterface.h
)

set(MEGAShellExt_SOURCES
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
    ClassicShellContextMenu/ContextMenuExt.cpp
    ContextMenuData.cpp
    ShellExt.cpp
    ShellExtNotASync.cpp
    RegUtils.cpp
    dllmain.cpp
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
    ${CMAKE_CURRENT_SOURCE_DIR}/ModernShellContextMenu/gui
    ${CMAKE_CURRENT_SOURCE_DIR}/ModernShellContextMenu/utilities
    ${CMAKE_CURRENT_SOURCE_DIR}/ModernShellContextMenu/msix
    ${CMAKE_CURRENT_SOURCE_DIR}/ClassicShellContextMenu
)

target_compile_features(MEGAShellExt PRIVATE cxx_std_17)

if(WIN32)
    # Generate Appxmanifest.xml
    if(CMAKE_SIZEOF_VOID_P EQUAL 8)
        set(DLL "ShellExtX64.dll")
    elseif(CMAKE_SIZEOF_VOID_P EQUAL 4)
        set(DLL "ShellExtX32.dll")
    endif()

    set(GenerateAppxManifestXmlScript "${CMAKE_CURRENT_SOURCE_DIR}/ModernShellContextMenu/msix/generateAppxManifest.cmake")
    add_custom_command(
        TARGET MEGAShellExt
        POST_BUILD
        COMMAND ${CMAKE_COMMAND} -DDLL=${DLL}
        -P ${GenerateAppxManifestXmlScript}
        COMMENT "Generating AppxManifest.xml"
    )
endif()
