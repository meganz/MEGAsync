##################################
#	qml targets sources     #
##################################

set(SOURCES ${SOURCES}
    ${CMAKE_CURRENT_SOURCE_DIR}/IQMLThemeTarget.h
    ${CMAKE_CURRENT_SOURCE_DIR}/QMLColorThemeTarget.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/QMLColorThemeTarget.h
    ${CMAKE_CURRENT_SOURCE_DIR}/QMLThemeTargetFactory.h
    ${CMAKE_CURRENT_SOURCE_DIR}/QMLTargetUtils.h
    ${CMAKE_CURRENT_SOURCE_DIR}/QMLTargetUtils.cpp    
    ${CMAKE_CURRENT_SOURCE_DIR}/QMLColorThemeManagerTarget.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/QMLColorThemeManagerTarget.h

    PARENT_SCOPE
)

set(INCLUDE_DIRECTORIES ${INCLUDE_DIRECTORIES}
    ${CMAKE_CURRENT_SOURCE_DIR}
    PARENT_SCOPE
)
