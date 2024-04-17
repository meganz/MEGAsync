cmake_minimum_required(VERSION 3.14)

project(DesignTokensImporter LANGUAGES CXX)

set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(QT NAMES Qt5 REQUIRED COMPONENTS Core Gui Xml)
find_package(Qt${QT_VERSION_MAJOR} REQUIRED COMPONENTS Core Gui Xml)

set(SOURCES ${SOURCES}
    main.cpp
    Utilities.cpp Utilities.h
    TokenManager.h TokenManager.cpp
    UIHandler.h UIHandler.cpp
    PathProvider.h PathProvider.cpp

    StylesheetParser.h StylesheetParser.cpp
    IThemeGenerator.h
    Types.h
)

set(INCLUDE_DIRECTORIES ${INCLUDE_DIRECTORIES}
    ${CMAKE_CURRENT_SOURCE_DIR}
)

add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/qml)
add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/qtwidget)

include_directories(${INCLUDE_DIRECTORIES})

add_executable(DesignTokensImporter ${SOURCES})

target_link_libraries(DesignTokensImporter
    Qt${QT_VERSION_MAJOR}::Core
    Qt${QT_VERSION_MAJOR}::Gui
    Qt${QT_VERSION_MAJOR}::Xml
)

include(GNUInstallDirs)
install(TARGETS DesignTokensImporter
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)
