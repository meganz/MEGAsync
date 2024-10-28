# CMakeLists.txt file to build the MEGA Desktop App, tests and utilities.

cmake_minimum_required(VERSION 3.18)

# Qt Creator configures VCPKG automatically. Disable it, we may want to use different tripplets, paths...
set(QT_CREATOR_SKIP_VCPKG_SETUP TRUE CACHE BOOL "")

## Modules location
list(APPEND CMAKE_MODULE_PATH ${CMAKE_CURRENT_LIST_DIR}/contrib/cmake/modules) # Modules from MEGA Desktop App
list(APPEND CMAKE_MODULE_PATH ${CMAKE_CURRENT_LIST_DIR}/src/MEGASync/mega/contrib/cmake/modules) # Modules from MEGAsdk

## Configurable options ##
set(VCPKG_ROOT "" CACHE PATH "If set, it will build and use the VCPKG packages defined in the manifest file")

# Load global CMake configuration for the project
include(desktopapp_initial_configuration)
include(desktopapp_options)

if(NOT PROJECT_NAME)
    if(VCPKG_ROOT)
        # Include VCPKG management tools.
        include(vcpkg_management)
        list(APPEND vcpkg_overlay ${CMAKE_CURRENT_LIST_DIR}/src/MEGASync/mega/contrib/cmake) # MEGAsdk overlays
        process_vcpkg_libraries("${vcpkg_overlay}") # Choose and build libraries depending on the configured options.
    else()
        # For packages with no pkg-config in the system.
        list(APPEND CMAKE_MODULE_PATH third-party/mega/contrib/cmake/modules/packages)
        message(STATUS "Using system dependencies")
    endif()
endif()

# Get MEGA Desktop App version to use it as the CMake project version.
include(desktopapp_load_version)
read_desktop_app_version(MEGA_DESKTOP_APP_VERSION ${CMAKE_CURRENT_LIST_DIR}/src/MEGASync/control/Version.h)

project(desktop
    VERSION ${MEGA_DESKTOP_APP_VERSION}
    DESCRIPTION "MEGA Desktop app project"
    )

# In-source build not allowed
if(CMAKE_SOURCE_DIR STREQUAL CMAKE_BINARY_DIR)
    message(FATAL_ERROR "In-source build is not allowed. Remove CMakeCache.txt and the CMakeFiles directory and set a new binary directory different than the source tree.")
endif()

if(ENABLE_DESKTOP_APP AND NOT ENABLE_QT_BINDINGS)
    message(FATAL_ERROR "Qt Destkop App requires Qt bindings to work. Turn on ENABLE_QT_BINDINGS option.")
endif()

if(ENABLE_DESKTOP_UPDATER AND (UNIX AND NOT APPLE))
    message(FATAL_ERROR "MEGAUpdater is not available for Linux. Turn off ENABLE_DESKTOP_UPDATER option.")
endif()

message(STATUS "Building MEGA Desktop App project v${PROJECT_VERSION}")

include(GNUInstallDirs) # Values for installation directories. All platforms
include(target_sources_conditional) # function to add files to the project without building them
include(target_platform_compile_options) # To add compile options depeding on the platform

# Load common and per platform configuration for the project
include(desktopapp_configuration)

# Load the MEGA targets
add_subdirectory(src)

include(get_clang_format)
get_clang_format()
