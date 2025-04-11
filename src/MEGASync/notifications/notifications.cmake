
set(DESKTOP_APP_NOTIFICATIONS_HEADERS
    ${CMAKE_CURRENT_LIST_DIR}/DesktopNotifications.h
    ${CMAKE_CURRENT_LIST_DIR}/TransferNotificationBuilder.h
    ${CMAKE_CURRENT_LIST_DIR}/NotificatorBase.h
    ${CMAKE_CURRENT_LIST_DIR}/NotificationDelayer.h
)

set(DESKTOP_APP_NOTIFICATIONS_SOURCES
    ${CMAKE_CURRENT_LIST_DIR}/DesktopNotifications.cpp
    ${CMAKE_CURRENT_LIST_DIR}/TransferNotificationBuilder.cpp
    ${CMAKE_CURRENT_LIST_DIR}/NotificatorBase.cpp
    ${CMAKE_CURRENT_LIST_DIR}/NotificationDelayer.cpp
)

target_sources_conditional(${ExecutableTarget}
   FLAG APPLE
   QT_AWARE
   PRIVATE
   ${CMAKE_CURRENT_LIST_DIR}/macx/NotificationHandler.mm
   ${CMAKE_CURRENT_LIST_DIR}/macx/UNUserNotificationHandler.mm
   ${CMAKE_CURRENT_LIST_DIR}/macx/UNUserNotificationDelegate.mm
   ${CMAKE_CURRENT_LIST_DIR}/macx/Notificator.cpp
   ${CMAKE_CURRENT_LIST_DIR}/macx/Notificator.h
   ${CMAKE_CURRENT_LIST_DIR}/macx/NotificationHandler.h
   ${CMAKE_CURRENT_LIST_DIR}/macx/UNUserNotificationHandler.h
   ${CMAKE_CURRENT_LIST_DIR}/macx/NotificationDelegate.h
   ${CMAKE_CURRENT_LIST_DIR}/macx/NotificationCategoryManager.h
   ${CMAKE_CURRENT_LIST_DIR}/macx/NotificationCategoryManager.mm
)

target_sources_conditional(${ExecutableTarget}
   FLAG WIN32
   QT_AWARE
   PRIVATE
   ${CMAKE_CURRENT_LIST_DIR}/win/Notificator.h
   ${CMAKE_CURRENT_LIST_DIR}/win/Notificator.cpp
)

target_sources_conditional(${ExecutableTarget}
    FLAG UNIX AND NOT APPLE
    QT_AWARE
    PRIVATE
    ${CMAKE_CURRENT_LIST_DIR}/linux/Notificator.h
    ${CMAKE_CURRENT_LIST_DIR}/linux/Notificator.cpp
)

target_sources(${ExecutableTarget}
    PRIVATE
    ${DESKTOP_APP_NOTIFICATIONS_HEADERS}
    ${DESKTOP_APP_NOTIFICATIONS_SOURCES}
)

if (APPLE)
    target_link_libraries(${ExecutableTarget}
        PRIVATE
        "-framework UserNotifications"
    )
endif()

if (UNIX AND NOT APPLE)
    target_include_directories(${ExecutableTarget}
        PUBLIC
        ${CMAKE_CURRENT_LIST_DIR}/linux
    )
endif()

set (INCLUDE_DIRECTORIES
    ${CMAKE_CURRENT_LIST_DIR}
)

target_include_directories(${ExecutableTarget}
    PUBLIC
    ${INCLUDE_DIRECTORIES}
    $<$<BOOL:${WIN32}>:${CMAKE_CURRENT_LIST_DIR}/win>
    $<$<BOOL:${APPLE}>:${CMAKE_CURRENT_LIST_DIR}/macx>
)
