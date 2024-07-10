cmake_minimum_required(VERSION 3.16.0)

project(MEGAShellExtNautilus LANGUAGES C)

find_package(PkgConfig REQUIRED)
pkg_search_module(NAUTILUS_EXT REQUIRED libnautilus-extension libnautilus-extension-4)

# Get the API version
pkg_get_variable(NAUTILUS_EXT_API_VERSION ${NAUTILUS_EXT_MODULE_NAME} extensions_api_version)

if(NOT NAUTILUS_EXT_API_VERSION)
    set(NAUTILUS_EXT_API_VERSION 1)
endif()

add_definitions(-DNAUTILUS_EXT_API_VERSION=${NAUTILUS_EXT_API_VERSION})

include_directories(${CMAKE_CURRENT_BINARY_DIR})

set(SOURCES
    mega_ext_module.c
    mega_ext_client.c
    mega_notify_client.c
    MEGAShellExt.c
)

set(HEADERS
    MEGAShellExt.h
    mega_ext_client.h
    mega_notify_client.h
)

# Create the library target
add_library(${PROJECT_NAME} SHARED ${SOURCES} ${HEADERS})
set_target_properties(${PROJECT_NAME} PROPERTIES
    PREFIX         "lib"
#    SOVERSION      "${PROJECT_VERSION}"
)

target_include_directories(${PROJECT_NAME} PRIVATE ${NAUTILUS_EXT_INCLUDE_DIRS})
target_link_libraries(${PROJECT_NAME} ${NAUTILUS_EXT_LIBRARIES})

# Get nautilus extensions path
pkg_get_variable(EXTENSIONS_PATH ${NAUTILUS_EXT_MODULE_NAME} extensiondir)

install(TARGETS ${PROJECT_NAME} LIBRARY DESTINATION ${EXTENSIONS_PATH})

# Install emblems
set(EMBLEMS_LOC ${CMAKE_CURRENT_SOURCE_DIR}/data/emblems)
set(FILES32
    ${EMBLEMS_LOC}/32x32/mega-pending.png
    ${EMBLEMS_LOC}/32x32/mega-pending.icon
    ${EMBLEMS_LOC}/32x32/mega-synced.icon
    ${EMBLEMS_LOC}/32x32/mega-synced.png
    ${EMBLEMS_LOC}/32x32/mega-syncing.icon
    ${EMBLEMS_LOC}/32x32/mega-syncing.png
    ${EMBLEMS_LOC}/32x32/mega-upload.icon
    ${EMBLEMS_LOC}/32x32/mega-upload.png
)

set(FILES64
    ${EMBLEMS_LOC}/64x64/mega-pending.png
    ${EMBLEMS_LOC}/64x64/mega-pending.icon
    ${EMBLEMS_LOC}/64x64/mega-synced.icon
    ${EMBLEMS_LOC}/64x64/mega-synced.png
    ${EMBLEMS_LOC}/64x64/mega-syncing.icon
    ${EMBLEMS_LOC}/64x64/mega-syncing.png
    ${EMBLEMS_LOC}/64x64/mega-upload.icon
    ${EMBLEMS_LOC}/64x64/mega-upload.png
)

set(HICOLOR share/icons/hicolor)
install(FILES ${FILES32} DESTINATION ${HICOLOR}/32x32/emblems)
install(FILES ${FILES64} DESTINATION ${HICOLOR}/64x64/emblems)

