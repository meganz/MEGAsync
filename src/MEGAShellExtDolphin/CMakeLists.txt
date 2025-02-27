cmake_minimum_required(VERSION 3.16.0)

project(megasync-plugin)

set(KF_VER "5" CACHE STRING "")
set(KF_DEP_VERSION "${KF_VER}")
set(Qt_VER ${KF_VER} CACHE STRING "")

include(FeatureSummary)
include(GenerateExportHeader)

set(CMAKE_AUTOMOC ON)

set(MEGA_PLUGIN_OVERLAY_SOURCES
    megasync-plugin-overlay.cpp
)

set(MEGA_PLUGIN_SOURCES
    megasync-plugin.h
    megasync-plugin.cpp
)

find_package(Qt${Qt_VER} CONFIG COMPONENTS
    Core
    Network
)

# ECM setup
find_package(ECM ${KF_DEP_VERSION} REQUIRED NO_MODULE)
set(CMAKE_MODULE_PATH ${ECM_MODULE_PATH})

if (${ECM_VERSION} VERSION_LESS 5.82.0)
    include(KDEInstallDirs)
else()
    include(KDEInstallDirs${KF_VER})
endif()
find_package(KF${KF_VER} REQUIRED COMPONENTS
    KIO
)
include(KDECMakeSettings NO_POLICY_SCOPE)
include(KDECompilerSettings NO_POLICY_SCOPE)

set_package_properties(MEGADolphinPlugin PROPERTIES
    DESCRIPTION "MEGA plugin for Dolphin"
    URL "http://mega.io"
    TYPE REQUIRED
    PURPOSE "Provides MEGA plugin for Dolphin."
)

add_definitions(-DQT_USE_FAST_CONCATENATION -DQT_USE_FAST_OPERATOR_PLUS)

#---OVERLAY PLUGIN---
set(MEGA_OVERLAYICON_PLUGIN megasync-overlay-plugin)
kcoreaddons_add_plugin(${MEGA_OVERLAYICON_PLUGIN}
    SOURCES ${MEGA_PLUGIN_OVERLAY_SOURCES}
    INSTALL_NAMESPACE "kf${KF_VER}/overlayicon"
)

target_link_libraries(${MEGA_OVERLAYICON_PLUGIN}
    KF${KF_VER}::KIOCore
    KF${KF_VER}::KIOWidgets
)

#---- ICONS ----
set(HICOLOR_INSTALL_DIR ${KDE_INSTALL_FULL_ICONDIR}/hicolor)
file(GLOB_RECURSE FILES32 "data/emblems/32x32/*")
file(GLOB_RECURSE FILES64 "data/emblems/64x64/*")
install(FILES ${FILES32} DESTINATION ${HICOLOR_INSTALL_DIR}/32x32/emblems)
install(FILES ${FILES64} DESTINATION ${HICOLOR_INSTALL_DIR}/64x64/emblems)

#---ACTION PLUGIN---
set(MEGA_PLUGIN megasync-plugin)
kcoreaddons_add_plugin(${MEGA_PLUGIN}
    SOURCES ${MEGA_PLUGIN_SOURCES}
    INSTALL_NAMESPACE "kf${KF_VER}/kfileitemaction"
)
target_link_libraries(${MEGA_PLUGIN}
    KF${KF_VER}::KIOCore
    KF${KF_VER}::KIOWidgets
    KF${KF_VER}::KIOFileWidgets
)
