
set(DESKTOP_APP_NOTIFICATIONS_HEADERS
    notifications/DesktopNotifications.h
    notifications/TransferNotificationBuilder.h
    notifications/NotificatorBase.h
    notifications/NotificationDelayer.h
)

set(DESKTOP_APP_NOTIFICATIONS_SOURCES
    notifications/DesktopNotifications.cpp
    notifications/TransferNotificationBuilder.cpp
    notifications/NotificatorBase.cpp
    notifications/NotificationDelayer.cpp
)

target_sources_conditional(MEGAsync
   FLAG APPLE
   QT_AWARE
   PRIVATE
   notifications/macx/NotificationHandler.mm
   notifications/macx/UNUserNotificationHandler.mm
   notifications/macx/NSUserNotificationHandler.mm
   notifications/macx/NSUserNotificationDelegate.mm
   notifications/macx/UNUserNotificationDelegate.mm
   notifications/macx/Notificator.cpp
   notifications/macx/Notificator.h
   notifications/macx/NotificationHandler.h
   notifications/macx/UNUserNotificationHandler.h
   notifications/macx/NotificationDelegate.h
   notifications/macx/NSUserNotificationHandler.h
)

target_sources_conditional(MEGAsync
   FLAG WIN32
   QT_AWARE
   PRIVATE
   notifications/win/Notificator.h
   notifications/win/Notificator.cpp   
)

target_sources_conditional(MEGAsync
    FLAG UNIX AND NOT APPLE
    QT_AWARE
    PRIVATE
    notifications/linux/Notificator.h
    notifications/linux/Notificator.cpp
)

target_sources(MEGAsync
    PRIVATE
    ${DESKTOP_APP_NOTIFICATIONS_HEADERS}
    ${DESKTOP_APP_NOTIFICATIONS_SOURCES}
)

if (APPLE)
    target_link_libraries(MEGAsync
        PRIVATE
        "-framework UserNotifications"
    )
endif()

if (UNIX AND NOT APPLE)
    target_include_directories(MEGAsync
        PUBLIC
        ${CMAKE_CURRENT_LIST_DIR}/linux
    )
endif()

set (INCLUDE_DIRECTORIES
    ${CMAKE_CURRENT_LIST_DIR}
)

target_include_directories(MEGAsync
    PUBLIC
    ${INCLUDE_DIRECTORIES}
    $<$<BOOL:${WIN32}>:${CMAKE_CURRENT_LIST_DIR}/win>
    $<$<BOOL:${APPLE}>:${CMAKE_CURRENT_LIST_DIR}/macx>
)
