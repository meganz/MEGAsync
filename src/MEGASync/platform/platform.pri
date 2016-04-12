DEPENDPATH += $$PWD
INCLUDEPATH += $$PWD

SOURCES += $$PWD/notificator.cpp
HEADERS +=  $$PWD/Platform.h $$PWD/notificator.h

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
	#VARIABLES
	isEmpty(PREFIX) {
	PREFIX = $$THE_RPM_BUILD_ROOT/usr
	}
	DATADIR =$$PREFIX/share
	
	distro.target = $$PWD/linux/data/distro
    distro.path = $$DATADIR/doc/megasync
    system(command -v lsb_release): distro.commands = lsb_release -ds > $$distro.target
    distro.files = $$distro.target
    
    version.target = $$PWD/linux/data/version
	version.path = $$DATADIR/doc/megasync
    system(command -v lsb_release): version.commands = lsb_release -rs > $$version.target
	version.files = $$version.target

	INSTALLS += distro version

    QT += dbus
    SOURCES += $$PWD/linux/LinuxPlatform.cpp \
        $$PWD/linux/ExtServer.cpp \
        $$PWD/linux/NotifyServer.cpp
    HEADERS += $$PWD/linux/LinuxPlatform.h \
        $$PWD/linux/ExtServer.h \
        $$PWD/linux/NotifyServer.h

    LIBS += -lssl -lcrypto
    DEFINES += USE_DBUS

    # do not install desktop files if no_desktop is defined,
    # make build tool take care of these files
    !contains(DEFINES, no_desktop) {

        message("Installing desktop files.")

        # get env variable
        DESKTOP_DESTDIR = $$(DESKTOP_DESTDIR)
        isEmpty(DESKTOP_DESTDIR) {
            DESKTOP_DESTDIR = /usr
        }

        # desktop
        desktop.path = $$DESKTOP_DESTDIR/share/applications
        desktop.files = $$PWD/linux/data/megasync.desktop
        desktop.commands = update-desktop-database &> /dev/null || true
        INSTALLS += desktop

        HICOLOR = $$DESKTOP_DESTDIR/share/icons/hicolor

        # icons
        ICONS_LOC = $$PWD/linux/data/icons/hicolor
        icons16.path = $${HICOLOR}/16x16/apps
        icons16.files = $${ICONS_LOC}/16x16/apps/mega.png
        icons32.path = $${HICOLOR}/32x32/apps
        icons32.files = $${ICONS_LOC}/32x32/apps/mega.png
        icons48.path = $${HICOLOR}/48x48/apps
        icons48.files = $${ICONS_LOC}/48x48/apps/mega.png
        icons128.path = $${HICOLOR}/128x128/apps
        icons128.files = $${ICONS_LOC}/128x128/apps/mega.png
        icons256.path = $${HICOLOR}/256x256/apps
        icons256.files = $${ICONS_LOC}/256x256/apps/mega.png
        INSTALLS += icons16 icons32 icons48 icons128 icons256
    } else {
        message("Skipping desktop files installation.")
    }
}

macx {
    SOURCES += $$PWD/macx/MacXPlatform.cpp

    HEADERS += $$PWD/macx/MacXPlatform.h \
        $$PWD/macx/MacXFunctions.h \
        $$PWD/macx/macnotificationhandler.h \
        $$PWD/macx/NotificationDelegate.h \
        $$PWD/macx/MacXSystemServiceTask.h  \
        $$PWD/macx/MEGAService.h

    OBJECTIVE_SOURCES += \
            $$PWD/macx/MacXFunctions.mm \
            $$PWD/macx/macnotificationhandler.mm \
            $$PWD/macx/NotificationDelegate.mm \
            $$PWD/macx/MacXSystemServiceTask.mm \
            $$PWD/macx/MEGAService.mm


    LIBS += -framework Cocoa
    LIBS += -framework Security
}
