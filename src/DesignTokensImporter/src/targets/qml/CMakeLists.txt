##################################
#	qml targets sources     #
##################################

set(DESIGN_TOKENS_IMPORTER_SOURCES ${DESIGN_TOKENS_IMPORTER_SOURCES}
    ${CMAKE_CURRENT_SOURCE_DIR}/QMLColorThemeTarget.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/QMLTargetUtils.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/QMLColorThemeManagerTarget.cpp

    PARENT_SCOPE
)

set(DESIGN_TOKENS_IMPORTER_HEADERS ${DESIGN_TOKENS_IMPORTER_HEADERS}
    ${CMAKE_CURRENT_SOURCE_DIR}/QMLColorThemeTarget.h
    ${CMAKE_CURRENT_SOURCE_DIR}/QMLTargetUtils.h
    ${CMAKE_CURRENT_SOURCE_DIR}/QMLColorThemeManagerTarget.h

    PARENT_SCOPE
)

set(INCLUDE_DIRECTORIES ${INCLUDE_DIRECTORIES}
    ${CMAKE_CURRENT_SOURCE_DIR}

    PARENT_SCOPE
)
