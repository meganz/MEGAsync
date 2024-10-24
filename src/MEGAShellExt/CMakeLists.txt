#
# MEGAShellExt
# Library responsible for the integration with windows explorer
#

add_library(MEGAShellExt SHARED)

set(MEGAShellExt_HEADERS
    contextmenu/data/ContextMenuData.h
    contextmenu/gui/AclHelper.h
    contextmenu/gui/ContextMenuCommandBase.h
    contextmenu/gui/ContextMenuCommandGetLink.h
    contextmenu/gui/ContextMenuCommandRemoveFromLeftPane.h
    contextmenu/gui/ContextMenuCommandSeparator.h
    contextmenu/gui/ContextMenuCommandUpload.h
    contextmenu/gui/ContextMenuCommandView.h
    contextmenu/gui/ContextMenuCommandViewVersions.h
    contextmenu/gui/framework.h
    contextmenu/gui/Installer.h
    contextmenu/gui/PathHelper.h
    contextmenu/gui/RegistryKey.h
    contextmenu/gui/SharedCounter.h
    contextmenu/gui/SharedState.h
    contextmenu/gui/SimpleFactory.h
    ShellExt.h
    ShellExtNotASync.h
    resource.h
    RegUtils.h
    ContextMenuExt.h
    ClassFactoryShellExtSyncing.h
    ClassFactoryShellExtSynced.h
    ClassFactoryShellExtPending.h
    ClassFactoryShellExtNotFound.h
    ClassFactoryContextMenuExt.h
    ClassFactory.h
    MegaInterface.h
)

set(MEGAShellExt_SOURCES
    contextmenu/data/ContextMenuData.cpp
    contextmenu/gui/AclHelper.cpp
    contextmenu/gui/ContextMenuCommandBase.cpp
    contextmenu/gui/ContextMenuCommandGetLink.cpp
    contextmenu/gui/ContextMenuCommandRemoveFromLeftPane.cpp
    contextmenu/gui/ContextMenuCommandSeparator.cpp
    contextmenu/gui/ContextMenuCommandUpload.cpp
    contextmenu/gui/ContextMenuCommandView.cpp
    contextmenu/gui/ContextMenuCommandViewVersions.cpp
    contextmenu/gui/Installer.cpp
    contextmenu/gui/PathHelper.cpp
    contextmenu/gui/RegistryKey.cpp
    contextmenu/gui/SharedCounter.cpp
    contextmenu/gui/SharedState.cpp
    ShellExt.cpp
    ShellExtNotASync.cpp
    RegUtils.cpp
    dllmain.cpp
    ContextMenuExt.cpp
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
    ${CMAKE_CURRENT_SOURCE_DIR}/contextmenu/data
    ${CMAKE_CURRENT_SOURCE_DIR}/contextmenu/gui
)

target_compile_features(MEGAShellExt PRIVATE cxx_std_17)
