##################################
#    qtwidget importer sources   #
##################################

add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/targets)

set(DESIGN_TOKENS_IMPORTER_SOURCES ${DESIGN_TOKENS_IMPORTER_SOURCES}
    ${CMAKE_CURRENT_SOURCE_DIR}/WidgetsDesignGenerator.h
    ${CMAKE_CURRENT_SOURCE_DIR}/WidgetsDesignGenerator.cpp

    PARENT_SCOPE
)

set(INCLUDE_DIRECTORIES ${INCLUDE_DIRECTORIES}
    ${CMAKE_CURRENT_SOURCE_DIR}

    PARENT_SCOPE
)
