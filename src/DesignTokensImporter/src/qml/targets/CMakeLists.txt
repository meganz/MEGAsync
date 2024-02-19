##################################
#	qml targets sources     #
##################################

set(SOURCES ${SOURCES}
    ${CMAKE_CURRENT_SOURCE_DIR}/IQMLStyleTarget.h
    ${CMAKE_CURRENT_SOURCE_DIR}/QMLColorStyleTarget.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/QMLColorStyleTarget.h
    ${CMAKE_CURRENT_SOURCE_DIR}/QMLStyleTargetFactory.h

    PARENT_SCOPE
)

set(INCLUDE_DIRECTORIES ${INCLUDE_DIRECTORIES}
    ${CMAKE_CURRENT_SOURCE_DIR}
    PARENT_SCOPE
)
