#
# MEGAShellExt
# Library responsible for the integration with windows explorer
#

add_library(MEGAShellExt SHARED)

set(MEGAShellExt_HEADERS
    ShellExt.h
    resource.h
    RegUtils.h
    ContextMenuExt.h
    ClassFactoryShellExtSyncing.h
    ClassFactoryShellExtSynced.h
    ClassFactoryShellExtPending.h
    ClassFactoryContextMenuExt.h
    ClassFactory.h
    MegaInterface.h
)

set(MEGAShellExt_SOURCES
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

