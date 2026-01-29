cmake_minimum_required(VERSION 3.16.0)
project(MEGAShellExtThunar LANGUAGES C)

find_package(PkgConfig REQUIRED)

# Try to find thunarx-2 first, then thunarx-3
pkg_check_modules(THUNARX thunarx-2)
if(THUNARX_FOUND)
    set(THUNARX_VERSION "2")
    add_definitions(-DUSING_THUNAR2)
    message(STATUS "Found thunarx-2")
else()
    pkg_check_modules(THUNARX thunarx-3)
    if(THUNARX_FOUND)
        set(THUNARX_VERSION "3")
        add_definitions(-DUSING_THUNAR3)
        message(STATUS "Found thunarx-3")
    else()
        message(WARNING "Thunar development files not found. Skipping MEGAShellExtThunar. Install libthunarx-2-dev or libthunarx-3-dev to enable.")
        return()
    endif()
endif()

# Find glib-2.0
pkg_check_modules(GLIB REQUIRED glib-2.0)

include_directories(${CMAKE_CURRENT_BINARY_DIR})

set(SOURCES
    MEGAShellExt.c
    mega_ext_client.c
)

set(HEADERS
    MEGAShellExt.h
    mega_ext_client.h
)

# Create the library target
add_library(${PROJECT_NAME} SHARED ${SOURCES} ${HEADERS})

set_target_properties(${PROJECT_NAME} PROPERTIES
    PREFIX         "lib"
)

target_include_directories(${PROJECT_NAME} PRIVATE 
    ${THUNARX_INCLUDE_DIRS}
    ${GLIB_INCLUDE_DIRS}
)

target_link_libraries(${PROJECT_NAME} 
    ${THUNARX_LIBRARIES}
    ${GLIB_LIBRARIES}
)

# Get Thunar extensions path
if(THUNARX_VERSION STREQUAL "2")
    pkg_get_variable(EXTENSIONS_PATH thunarx-2 extensionsdir)
else()
    pkg_get_variable(EXTENSIONS_PATH thunarx-3 extensionsdir)
endif()

if(NOT EXTENSIONS_PATH)
    message(WARNING "Could not determine Thunar extensions directory, using default")
    set(EXTENSIONS_PATH "lib/thunarx-${THUNARX_VERSION}")
endif()

message(STATUS "Thunar extensions will be installed to: ${EXTENSIONS_PATH}")

install(TARGETS ${PROJECT_NAME} LIBRARY DESTINATION ${EXTENSIONS_PATH})
