DEPENDPATH += $$PWD
INCLUDEPATH += $$PWD

SOURCES += $$PWD/AbstractPlatform.cpp \
    $$PWD/Platform.cpp \
    $$PWD/ShellNotifier.cpp

HEADERS +=  $$PWD/Platform.h \
            $$PWD/AbstractPlatform.h \
            $$PWD/ShellNotifier.h \
            $$PWD/PowerOptions.h \
            $$PWD/PlatformStrings.h
win32 {
    SOURCES +=	$$PWD/win/PlatformImplementation.cpp \
    $$PWD/win/RecursiveShellNotifier.cpp \
    $$PWD/win/ThreadedQueueShellNotifier.cpp \
		$$PWD/win/WinShellDispatcherTask.cpp \
                $$PWD/win/WinTrayReceiver.cpp \
                $$PWD/win/wintoastlib.cpp \
                $$PWD/win/PowerOptions.cpp \
                $$PWD/win/PlatformStrings.cpp

    HEADERS  += $$PWD/win/PlatformImplementation.h \
    $$PWD/win/RecursiveShellNotifier.h \
    $$PWD/win/ThreadedQueueShellNotifier.h \
		$$PWD/win/WinShellDispatcherTask.h \
                $$PWD/win/WinTrayReceiver.h \
                $$PWD/win/wintoastlib.h \
                $$PWD/win/WintoastCompat.h \
                $$PWD/win/WinAPIShell.h

    LIBS += -lole32 -lShell32 -lcrypt32 -ltaskschd -lPowrprof
    DEFINES += UNICODE _UNICODE NTDDI_VERSION=0x06010000 _WIN32_WINNT=0x0601
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

    SOURCES += $$PWD/linux/PlatformImplementation.cpp \
        $$PWD/linux/ExtServer.cpp \
        $$PWD/linux/NotifyServer.cpp \
        $$PWD/linux/PowerOptions.cpp \
        $$PWD/linux/PlatformStrings.cpp
    HEADERS += $$PWD/linux/PlatformImplementation.h \
        $$PWD/linux/ExtServer.h \
        $$PWD/linux/NotifyServer.h

    LIBS += -lssl -lcrypto -ldl -lxcb
    DEFINES += USE_DBUS
    contains(DEFINES, USE_DBUS)
    {
        QT += dbus
    }

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

        trayiconssynching.extra = -\$(INSTALL_FILE) $$PWD/../gui/images/synching.svg \$(INSTALL_ROOT)$${HICOLOR}/scalable/status/megasynching.svg
        trayiconswarning.extra = -\$(INSTALL_FILE) $$PWD/../gui/images/warning.svg \$(INSTALL_ROOT)$${HICOLOR}/scalable/status/megawarning.svg
        trayiconsalert.extra = -\$(INSTALL_FILE) $$PWD/../gui/images/alert.svg \$(INSTALL_ROOT)$${HICOLOR}/scalable/status/megaalert.svg
        trayiconspaused.extra = -\$(INSTALL_FILE) $$PWD/../gui/images/paused.svg \$(INSTALL_ROOT)$${HICOLOR}/scalable/status/megapaused.svg
        trayiconslogging.extra = -\$(INSTALL_FILE) $$PWD/../gui/images/logging.svg \$(INSTALL_ROOT)$${HICOLOR}/scalable/status/megalogging.svg
        trayiconsuptodate.extra = -\$(INSTALL_FILE) $$PWD/../gui/images/uptodate.svg \$(INSTALL_ROOT)$${HICOLOR}/scalable/status/megauptodate.svg
        trayiconssynching.extra = -\$(INSTALL_FILE) $$PWD/../gui/images/synching.svg \$(INSTALL_ROOT)$${HICOLOR}/scalable/status/megasynching.svg

        trayiconssynching.path = $${HICOLOR}/scalable/status
        trayiconswarning.path = $${HICOLOR}/scalable/status
        trayiconsalert.path = $${HICOLOR}/scalable/status
        trayiconspaused.path = $${HICOLOR}/scalable/status
        trayiconslogging.path = $${HICOLOR}/scalable/status
        trayiconsuptodate.path = $${HICOLOR}/scalable/status

        trayiconssynching.uninstall = -\$(DEL_DIR) \$(INSTALL_ROOT)$${HICOLOR}/scalable/status/megasynching.svg
        trayiconswarning.uninstall = -\$(DEL_DIR) \$(INSTALL_ROOT)$${HICOLOR}/scalable/status/megawarning.svg
        trayiconsalert.uninstall = -\$(DEL_DIR) \$(INSTALL_ROOT)$${HICOLOR}/scalable/status/megaalert.svg
        trayiconspaused.uninstall = -\$(DEL_DIR) \$(INSTALL_ROOT)$${HICOLOR}/scalable/status/megapaused.svg
        trayiconslogging.uninstall = -\$(DEL_DIR) \$(INSTALL_ROOT)$${HICOLOR}/scalable/status/megalogging.svg
        trayiconsuptodate.uninstall = -\$(DEL_DIR) \$(INSTALL_ROOT)$${HICOLOR}/scalable/status/megauptodate.svg

        INSTALLS += trayiconssynching trayiconswarning trayiconsalert trayiconspaused trayiconslogging trayiconsuptodate

        MONOCOLOR = $$DESKTOP_DESTDIR/share/icons/ubuntu-mono-dark

        trayiconssynchingmono.extra = -\$(INSTALL_FILE) $$PWD/../gui/images/synching_clear.svg \$(INSTALL_ROOT)$${MONOCOLOR}/status/24/megasynching.svg
        trayiconswarningmono.extra = -\$(INSTALL_FILE) $$PWD/../gui/images/warning_clear.svg \$(INSTALL_ROOT)$${MONOCOLOR}/status/24/megawarning.svg
        trayiconsalertmono.extra = -\$(INSTALL_FILE) $$PWD/../gui/images/alert_clear.svg \$(INSTALL_ROOT)$${MONOCOLOR}/status/24/megaalert.svg
        trayiconspausedmono.extra = -\$(INSTALL_FILE) $$PWD/../gui/images/paused_clear.svg \$(INSTALL_ROOT)$${MONOCOLOR}/status/24/megapaused.svg
        trayiconsloggingmono.extra = -\$(INSTALL_FILE) $$PWD/../gui/images/logging_clear.svg \$(INSTALL_ROOT)$${MONOCOLOR}/status/24/megalogging.svg
        trayiconsuptodatemono.extra = -\$(INSTALL_FILE) $$PWD/../gui/images/uptodate_clear.svg \$(INSTALL_ROOT)$${MONOCOLOR}/status/24/megauptodate.svg
        trayiconssynchingmono.extra = -\$(INSTALL_FILE) $$PWD/../gui/images/synching_clear.svg \$(INSTALL_ROOT)$${MONOCOLOR}/status/24/megasynching.svg

        trayiconssynchingmono.path = $${MONOCOLOR}/status/24
        trayiconswarningmono.path = $${MONOCOLOR}/status/24
        trayiconsalertmono.path = $${MONOCOLOR}/status/24
        trayiconspausedmono.path = $${MONOCOLOR}/status/24
        trayiconsloggingmono.path = $${MONOCOLOR}/status/24
        trayiconsuptodatemono.path = $${MONOCOLOR}/status/24

        trayiconssynchingmono.uninstall = -\$(DEL_DIR) \$(INSTALL_ROOT)$${MONOCOLOR}/status/24/megasynching.svg
        trayiconswarningmono.uninstall = -\$(DEL_DIR) \$(INSTALL_ROOT)$${MONOCOLOR}/status/24/megawarning.svg
        trayiconsalertmono.uninstall = -\$(DEL_DIR) \$(INSTALL_ROOT)$${MONOCOLOR}/status/24/megaalert.svg
        trayiconspausedmono.uninstall = -\$(DEL_DIR) \$(INSTALL_ROOT)$${MONOCOLOR}/status/24/megapaused.svg
        trayiconsloggingmono.uninstall = -\$(DEL_DIR) \$(INSTALL_ROOT)$${MONOCOLOR}/status/24/megalogging.svg
        trayiconsuptodatemono.uninstall = -\$(DEL_DIR) \$(INSTALL_ROOT)$${MONOCOLOR}/status/24/megauptodate.svg

        INSTALLS += trayiconssynchingmono trayiconswarningmono trayiconsalertmono trayiconspausedmono trayiconsloggingmono trayiconsuptodatemono

    } else {
        message("Skipping desktop files installation.")
    }
}

macx {
    SOURCES += $$PWD/macx/PlatformImplementation.cpp \
        $$PWD/macx/MacXExtServerService.cpp \
        $$PWD/macx/PlatformStrings.cpp

    HEADERS += $$PWD/macx/PlatformImplementation.h \
        $$PWD/macx/MacXFunctions.h \
        $$PWD/macx/MacXSystemServiceTask.h  \
        $$PWD/macx/MEGAService.h \
        $$PWD/macx/ClientSide.h \
        $$PWD/macx/ServerSide.h \
        $$PWD/macx/MacXExtServer.h \
        $$PWD/macx/MacXLocalServer.h \
        $$PWD/macx/MacXLocalServerPrivate.h \
        $$PWD/macx/MacXLocalSocket.h \
        $$PWD/macx/MacXLocalSocketPrivate.h \
        $$PWD/macx/NSPopover+MISSINGBackgroundView.h \
        $$PWD/macx/LockedPopOver.h \
        $$PWD/macx/Protocol.h \
        $$PWD/macx/MacXExtServerService.h \
        $$PWD/macx/QCustomMacToolbar.h \
        $$PWD/macx/NativeMacPopover.h \
        $$PWD/macx/NativeMacPopoverPrivate.h


    OBJECTIVE_SOURCES += \
            $$PWD/macx/MacXFunctions.mm \
            $$PWD/macx/MacXSystemServiceTask.mm \
            $$PWD/macx/MEGAService.mm \
            $$PWD/macx/ClientSide.mm \
            $$PWD/macx/ServerSide.mm \
            $$PWD/macx/MacXExtServer.mm \
            $$PWD/macx/MacXLocalServer.mm \
            $$PWD/macx/MacXLocalServerPrivate.mm \
            $$PWD/macx/MacXLocalSocket.mm \
            $$PWD/macx/MacXLocalSocketPrivate.mm \
            $$PWD/macx/NSPopover+MISSINGBackgroundView.mm \
            $$PWD/macx/LockedPopOver.mm \
            $$PWD/macx/QCustomMacToolbar.mm \
            $$PWD/macx/PowerOptions.mm \
            $$PWD/macx/NativeMacPopover.mm \
            $$PWD/macx/NativeMacPopoverPrivate.mm

    LIBS += -framework Cocoa
    LIBS += -framework Security
    LIBS += -framework UserNotifications
}
