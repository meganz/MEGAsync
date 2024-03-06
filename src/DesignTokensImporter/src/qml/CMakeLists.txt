##################################
#	qml importer sources     #
##################################

add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/targets)

set(SOURCES ${SOURCES}
    ${CMAKE_CURRENT_SOURCE_DIR}/QMLStyleGenerator.h
    ${CMAKE_CURRENT_SOURCE_DIR}/QMLStyleGenerator.cpp

    PARENT_SCOPE
)

set(INCLUDE_DIRECTORIES ${INCLUDE_DIRECTORIES}
    ${CMAKE_CURRENT_SOURCE_DIR}
    PARENT_SCOPE
)
