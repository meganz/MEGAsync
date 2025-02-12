
set(DESKTOP_APP_UA_REQUEST_HEADERS
    ${CMAKE_CURRENT_LIST_DIR}/Avatar.h
    ${CMAKE_CURRENT_LIST_DIR}/CameraUploadFolder.h
    ${CMAKE_CURRENT_LIST_DIR}/DeviceName.h
    ${CMAKE_CURRENT_LIST_DIR}/FullName.h
    ${CMAKE_CURRENT_LIST_DIR}/MyBackupsHandle.h
    ${CMAKE_CURRENT_LIST_DIR}/MyChatFilesFolder.h
)

set(DESKTOP_APP_UA_REQUEST_SOURCES
    ${CMAKE_CURRENT_LIST_DIR}/Avatar.cpp
    ${CMAKE_CURRENT_LIST_DIR}/CameraUploadFolder.cpp
    ${CMAKE_CURRENT_LIST_DIR}/DeviceName.cpp
    ${CMAKE_CURRENT_LIST_DIR}/FullName.cpp
    ${CMAKE_CURRENT_LIST_DIR}/MyBackupsHandle.cpp
    ${CMAKE_CURRENT_LIST_DIR}/MyChatFilesFolder.cpp
)

target_sources(${ExecutableTarget}
    PRIVATE
    ${DESKTOP_APP_UA_REQUEST_HEADERS}
    ${DESKTOP_APP_UA_REQUEST_SOURCES}
)

set (INCLUDE_DIRECTORIES
    ${CMAKE_CURRENT_LIST_DIR}
)

target_include_directories(${ExecutableTarget} PRIVATE ${INCLUDE_DIRECTORIES})

