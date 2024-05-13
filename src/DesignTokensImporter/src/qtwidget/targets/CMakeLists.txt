##################################
#   qtwidget targets sources     #
##################################

set(DESIGN_TOKENS_IMPORTER_SOURCES ${DESIGN_TOKENS_IMPORTER_SOURCES}
    ${CMAKE_CURRENT_SOURCE_DIR}/WidgetsDesignTarget.h
    ${CMAKE_CURRENT_SOURCE_DIR}/WidgetsColorDesignTarget.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/WidgetsColorDesignTarget.h
    ${CMAKE_CURRENT_SOURCE_DIR}/WidgetsDesignTargetFactory.h

    PARENT_SCOPE
)

set(INCLUDE_DIRECTORIES ${INCLUDE_DIRECTORIES}
    ${CMAKE_CURRENT_SOURCE_DIR}

    PARENT_SCOPE
)
