cmake_minimum_required(VERSION 3.16.0)
project(MEGAShellExtNemo LANGUAGES C)

find_package(PkgConfig REQUIRED)
pkg_search_module(NEMO_EXT REQUIRED libnemo-extension)

# Get the API version (if available)
pkg_get_variable(NEMO_EXT_API_VERSION ${NEMO_EXT_MODULE_NAME} extensions_api_version)
if(NOT NEMO_EXT_API_VERSION)
    set(NEMO_EXT_API_VERSION 1)
endif()
add_definitions(-DNEMO_EXT_API_VERSION=${NEMO_EXT_API_VERSION})

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
)

target_include_directories(${PROJECT_NAME} PRIVATE ${NEMO_EXT_INCLUDE_DIRS})
target_link_libraries(${PROJECT_NAME} ${NEMO_EXT_LIBRARIES})

# Get nemo extensions path
pkg_get_variable(EXTENSIONS_PATH ${NEMO_EXT_MODULE_NAME} extensiondir)
install(TARGETS ${PROJECT_NAME} LIBRARY DESTINATION ${EXTENSIONS_PATH})

# Install emblems (nemo-specific)
set(EMBLEMS_LOC ${CMAKE_CURRENT_SOURCE_DIR}/data/emblems)

set(FILES32
    ${EMBLEMS_LOC}/32x32/mega-pending-nemo.png
    ${EMBLEMS_LOC}/32x32/mega-pending-nemo.icon
    ${EMBLEMS_LOC}/32x32/mega-synced-nemo.icon
    ${EMBLEMS_LOC}/32x32/mega-synced-nemo.png
    ${EMBLEMS_LOC}/32x32/mega-syncing-nemo.icon
    ${EMBLEMS_LOC}/32x32/mega-syncing-nemo.png
    ${EMBLEMS_LOC}/32x32/mega-upload-nemo.icon
    ${EMBLEMS_LOC}/32x32/mega-upload-nemo.png
)

set(FILES64
    ${EMBLEMS_LOC}/64x64/mega-pending-nemo.png
    ${EMBLEMS_LOC}/64x64/mega-pending-nemo.icon
    ${EMBLEMS_LOC}/64x64/mega-synced-nemo.icon
    ${EMBLEMS_LOC}/64x64/mega-synced-nemo.png
    ${EMBLEMS_LOC}/64x64/mega-syncing-nemo.icon
    ${EMBLEMS_LOC}/64x64/mega-syncing-nemo.png
    ${EMBLEMS_LOC}/64x64/mega-upload-nemo.icon
    ${EMBLEMS_LOC}/64x64/mega-upload-nemo.png
)

set(HICOLOR share/icons/hicolor)
install(FILES ${FILES32} DESTINATION ${HICOLOR}/32x32/emblems)
install(FILES ${FILES64} DESTINATION ${HICOLOR}/64x64/emblems)
