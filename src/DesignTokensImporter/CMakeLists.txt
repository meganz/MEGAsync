#
# Design tokens importer
# Binary to import design resources from the megadesignassets repository to be
# used by the MEGASync desktop app.
#
set(CMAKE_AUTOMOC ON)

add_executable(DesignTokensImporter)
set_target_properties(DesignTokensImporter PROPERTIES RUNTIME_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/src/DesignTokensImporter)

find_package(QT NAMES Qt5 REQUIRED COMPONENTS Core Gui Xml)
find_package(Qt${QT_VERSION_MAJOR} REQUIRED COMPONENTS Core Gui Xml)

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
    Qt${QT_VERSION_MAJOR}::Gui
    Qt${QT_VERSION_MAJOR}::Xml
)
