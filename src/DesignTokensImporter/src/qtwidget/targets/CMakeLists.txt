##################################
#   qtwidget targets sources     #
##################################

set(DESIGN_TOKENS_IMPORTER_SOURCES ${DESIGN_TOKENS_IMPORTER_SOURCES}
    ${CMAKE_CURRENT_SOURCE_DIR}/IQTWIDGETStyleTarget.h
    ${CMAKE_CURRENT_SOURCE_DIR}/QTWIDGETColorStyleTarget.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/QTWIDGETColorStyleTarget.h
    ${CMAKE_CURRENT_SOURCE_DIR}/QTWIDGETStyleTargetFactory.h

    PARENT_SCOPE
)

set(INCLUDE_DIRECTORIES ${INCLUDE_DIRECTORIES}
    ${CMAKE_CURRENT_SOURCE_DIR}

    PARENT_SCOPE
)
