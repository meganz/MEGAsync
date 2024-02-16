cmake_minimum_required(VERSION 3.14)

project(DesignTokensImporter LANGUAGES CXX)

set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(QT NAMES Qt5 REQUIRED COMPONENTS Core Gui Svg Xml)
find_package(Qt${QT_VERSION_MAJOR} REQUIRED COMPONENTS Core Gui Svg Xml)

add_executable(DesignTokensImporter
  main.cpp
  utilities.cpp utilities.h
  TokenManager.h TokenManager.cpp
  UIClass.h UIClass.cpp
  SVGIcon.h SVGIcon.cpp
  SVGRenderer.h SVGRenderer.cpp
  PathProvider.h PathProvider.cpp
  Resources.qrc
  StylesheetParser.h StylesheetParser.cpp
  StyleDefinitions.h
)
target_link_libraries(DesignTokensImporter
    Qt${QT_VERSION_MAJOR}::Core
    Qt${QT_VERSION_MAJOR}::Gui
    Qt${QT_VERSION_MAJOR}::Svg
    Qt${QT_VERSION_MAJOR}::Xml
)

include(GNUInstallDirs)
install(TARGETS DesignTokensImporter
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)
