#
# Design tokens importer
# Binary to import design resources from the megadesignassets repository to be
# used by the MEGASync desktop app.
#
cmake_minimum_required(VERSION 3.18)

project(DesignTokensImporter
    DESCRIPTION "Binary to import design resources from the megadesignassets repository to be used by the MEGASync desktop app."
)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(CMAKE_AUTOMOC ON)

include(FetchContent)
FetchContent_Declare(
    megadesignassets
    GIT_REPOSITORY git@code.developers.mega.co.nz:megadesignassets/megadesignassets.git
    SOURCE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/megadesignassets"
    GIT_TAG main  # Branch, tag, or commit
)
FetchContent_MakeAvailable(megadesignassets)

add_executable(DesignTokensImporter)
set_target_properties(DesignTokensImporter PROPERTIES RUNTIME_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR})

find_package(QT NAMES Qt5 REQUIRED COMPONENTS Core Xml)
find_package(Qt${QT_VERSION_MAJOR} REQUIRED COMPONENTS Core Xml)

set(DESIGN_TOKENS_IMPORTER_SOURCES)
set(DESIGN_TOKENS_IMPORTER_HEADERS)
set(INCLUDE_DIRECTORIES)

add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/src)

include_directories(${INCLUDE_DIRECTORIES})

target_sources(DesignTokensImporter
    PRIVATE
    ${DESIGN_TOKENS_IMPORTER_SOURCES}
    ${DESIGN_TOKENS_IMPORTER_HEADERS}
)

target_link_libraries(DesignTokensImporter
    Qt${QT_VERSION_MAJOR}::Core
    Qt${QT_VERSION_MAJOR}::Xml
)
