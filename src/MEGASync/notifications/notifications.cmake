
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
   PRIVATE
   notifications/../gui/Resources_macx.qrc
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
   PRIVATE
   notifications/../gui/Resources_win.qrc
   notifications/win/Notificator.h
   notifications/win/Notificator.cpp   
)

target_sources(MEGAsync
    PRIVATE
    ${DESKTOP_APP_NOTIFICATIONS_HEADERS}
    ${DESKTOP_APP_NOTIFICATIONS_SOURCES}
)

if (WIN32)
    target_include_directories(MEGAsync PRIVATE ${CMAKE_CURRENT_LIST_DIR}
        ${CMAKE_CURRENT_LIST_DIR}/win
    )
elseif (APPLE)
    target_link_libraries(MEGAsync
        PRIVATE
        "-framework UserNotifications"
    )

    target_include_directories(MEGAsync PRIVATE ${CMAKE_CURRENT_LIST_DIR}
        ${CMAKE_CURRENT_LIST_DIR}/macx
    )
else()

endif()


