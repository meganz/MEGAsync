DEPENDPATH += $$PWD
INCLUDEPATH += $$PWD

SOURCES += $$PWD/DesktopNotifications.cpp \
           $$PWD/TransferNotificationBuilder.cpp \
           $$PWD/NotificatorBase.cpp \
           $$PWD/RemovedSharesNotificator.cpp

HEADERS += $$PWD/DesktopNotifications.h \
           $$PWD/TransferNotificationBuilder.h  \
           $$PWD/NotificatorBase.h \
           $$PWD/RemovedSharesNotificator.h

win32 {
    RESOURCES += $$PWD/../gui/Resources_win.qrc
    INCLUDEPATH += $$PWD/win
    SOURCES += $$PWD/win/Notificator.cpp
    HEADERS += $$PWD/win/Notificator.h
}

macx {
    RESOURCES += $$PWD/../gui/Resources_macx.qrc
    INCLUDEPATH += $$PWD/macx

    OBJECTIVE_SOURCES += $$PWD/macx/NotificationHandler.mm \
           $$PWD/macx/UNUserNotificationHandler.mm \
           $$PWD/macx/NSUserNotificationHandler.mm \
           $$PWD/macx/NSUserNotificationDelegate.mm \
           $$PWD/macx/UNUserNotificationDelegate.mm

    SOURCES += $$PWD/macx/Notificator.cpp

    HEADERS += $$PWD/macx/Notificator.h \
               $$PWD/macx/NotificationHandler.h \
               $$PWD/macx/UNUserNotificationHandler.h \
               $$PWD/macx/NotificationDelegate.h \
               $$PWD/macx/NSUserNotificationHandler.h

    LIBS += -framework Cocoa
    LIBS += -framework Security
    LIBS += -framework UserNotifications
}

unix:!macx {
    RESOURCES += $$PWD/../gui/Resources_linux.qrc
    INCLUDEPATH += $$PWD/linux
    SOURCES += $$PWD/linux/Notificator.cpp
    HEADERS += $$PWD/linux/Notificator.h
}
