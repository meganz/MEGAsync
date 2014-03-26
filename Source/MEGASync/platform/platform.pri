DEPENDPATH += $$PWD
INCLUDEPATH += $$PWD

HEADERS  +=  $$PWD/Platform.h

win32 {
    SOURCES +=	$$PWD/win/WindowsPlatform.cpp \
		$$PWD/win/WinShellDispatcherTask.cpp \
		$$PWD/win/WinTrayReceiver.cpp

    HEADERS  += $$PWD/win/WindowsPlatform.h \
		$$PWD/win/WinShellDispatcherTask.h \
		$$PWD/win/WinTrayReceiver.h

    LIBS += -lole32 -lShell32 -lcrypt32
    DEFINES += -DUNICODE -DNTDDI_VERSION=0x05010000 -D_WIN32_WINNT=0x0501 -DWIN32_LEAN_AND_MEAN
}

unix:!macx {
    SOURCES += $$PWD/linux/LinuxPlatform.cpp \
        $$PWD/linux/ExtServer.cpp
    HEADERS += $$PWD/linux/LinuxPlatform.h \
        $$PWD/linux/ExtServer.h

    # desktop
    desktop.path = /usr/share/applications
    desktop.files =  $$PWD/linux/megasync.desktop
    INSTALLS += desktop
}

macx {
    SOURCES += $$PWD/macx/MacXPlatform.cpp
    HEADERS += $$PWD/macx/MacXPlatform.h
}
