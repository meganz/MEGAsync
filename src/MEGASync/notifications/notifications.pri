DEPENDPATH += $$PWD
INCLUDEPATH += $$PWD

SOURCES += $$PWD/DesktopNotifications.cpp \
           $$PWD/TransferNotificationBuilderBase.cpp \
           $$PWD/NotificatorBase.cpp \
           $$PWD/RemovedSharesNotificator.cpp

HEADERS += $$PWD/DesktopNotifications.h \
           $$PWD/TransferNotificationBuilderBase.h  \
           $$PWD/NotificatorBase.h \
           $$PWD/RemovedSharesNotificator.h

win32 {
    RESOURCES += $$PWD/../gui/Resources_win.qrc
    INCLUDEPATH += $$PWD/win
    SOURCES += $$PWD/win/TransferNotificationBuilder.cpp \
               $$PWD/win/Notificator.cpp
    HEADERS += $$PWD/win/TransferNotificationBuilder.h \
               $$PWD/win/Notificator.h
}

macx {
    RESOURCES += $$PWD/../gui/Resources_macx.qrc
    INCLUDEPATH += $$PWD/macx

    OBJECTIVE_SOURCES += $$PWD/macx/NotificationHandler.mm \
           $$PWD/macx/UNUserNotificationHandler.mm \
           $$PWD/macx/NSUserNotificationHandler.mm \
           $$PWD/macx/NSUserNotificationDelegate.mm \
           $$PWD/macx/UNUserNotificationDelegate.mm

    SOURCES += $$PWD/macx/TransferNotificationBuilder.cpp \
               $$PWD/macx/Notificator.cpp

    HEADERS += $$PWD/macx/TransferNotificationBuilder.h \
               $$PWD/macx/Notificator.h \
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
    SOURCES += $$PWD/linux/TransferNotificationBuilder.cpp\
               $$PWD/linux/Notificator.cpp
    HEADERS += $$PWD/linux/TransferNotificationBuilder.h\
               $$PWD/linux/Notificator.h
}
