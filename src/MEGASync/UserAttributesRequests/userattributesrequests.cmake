
set(DESKTOP_APP_UA_REQUEST_HEADERS
    UserAttributesRequests/Avatar.h
    UserAttributesRequests/CameraUploadFolder.h
    UserAttributesRequests/DeviceName.h
    UserAttributesRequests/FullName.h
    UserAttributesRequests/MyBackupsHandle.h
    UserAttributesRequests/MyChatFilesFolder.h
)

set(DESKTOP_APP_UA_REQUEST_SOURCES
    UserAttributesRequests/Avatar.cpp
    UserAttributesRequests/CameraUploadFolder.cpp
    UserAttributesRequests/DeviceName.cpp
    UserAttributesRequests/FullName.cpp
    UserAttributesRequests/MyBackupsHandle.cpp
    UserAttributesRequests/MyChatFilesFolder.cpp
)

target_sources(MEGAsync
    PRIVATE
    ${DESKTOP_APP_UA_REQUEST_HEADERS}
    ${DESKTOP_APP_UA_REQUEST_SOURCES}
)
