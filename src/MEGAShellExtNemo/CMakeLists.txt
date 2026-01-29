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

# Determine install prefix from env vble
if(DEFINED ENV{DESKTOP_DESTDIR})
    set(DESKTOP_DESTDIR "$ENV{DESKTOP_DESTDIR}")
else()
    set(DESKTOP_DESTDIR "/usr")
endif()

# Get nemo extensions path
pkg_get_variable(EXTENSIONS_PATH ${NEMO_EXT_MODULE_NAME} extensiondir)

# Strip /usr
string(REGEX REPLACE "^/usr" "" RELATIVE_EXTENSIONS_PATH "${EXTENSIONS_PATH}")
set(FINAL_EXT_PATH "${DESKTOP_DESTDIR}${RELATIVE_EXTENSIONS_PATH}")

install(TARGETS ${PROJECT_NAME} LIBRARY DESTINATION "${FINAL_EXT_PATH}")

#Clean up
set_directory_properties(PROPERTIES ADDITIONAL_MAKE_CLEAN_FILES
    "lib${PROJECT_NAME}.so;lib${PROJECT_NAME}.so.1;lib${PROJECT_NAME}.so.1.0")

# Install emblems (nemo-specific)
set(EMBLEMS_LOC "${CMAKE_CURRENT_SOURCE_DIR}/data/emblems")
set(HICOLOR "${DESKTOP_DESTDIR}/share/icons/hicolor")

install(DIRECTORY "${EMBLEMS_LOC}/32x32/" DESTINATION "${HICOLOR}/32x32/emblems" FILES_MATCHING PATTERN "*nemo*")
install(DIRECTORY "${EMBLEMS_LOC}/64x64/" DESTINATION "${HICOLOR}/64x64/emblems" FILES_MATCHING PATTERN "*nemo*")

if(NOT DEFINED no_desktop)
    install(CODE "
        message(STATUS \"Updating GTK\")
        execute_process(COMMAND gtk-update-icon-cache -f -t ${HICOLOR})
        ")
endif()
