
QT       += network

SOURCES += gui/SettingsDialog.cpp \
    gui/ActiveTransfer.cpp \
    gui/InfoDialog.cpp \
    gui/RecentFile.cpp \
    gui/TransferProgressBar.cpp \
    gui/UsageProgressBar.cpp \
    gui/SetupWizard.cpp \
    gui/NodeSelector.cpp \
    gui/FolderBinder.cpp \
    gui/BindFolderDialog.cpp \
    gui/UploadToMegaDialog.cpp \
    gui/PasteMegaLinksDialog.cpp \
    gui/ImportMegaLinksDialog.cpp \
    gui/ImportListWidgetItem.cpp \
    gui/CrashReportDialog.cpp \
    gui/MultiQFileDialog.cpp \
    gui/MegaProxyStyle.cpp

HEADERS  += gui/SettingsDialog.h \
    gui/ActiveTransfer.h \
    gui/InfoDialog.h \
    gui/RecentFile.h \
    gui/TransferProgressBar.h \
    gui/UsageProgressBar.h \
    gui/SetupWizard.h \
    gui/NodeSelector.h \
    gui/FolderBinder.h \
    gui/BindFolderDialog.h \
    gui/UploadToMegaDialog.h \
    gui/PasteMegaLinksDialog.h \
    gui/ImportMegaLinksDialog.h \
    gui/ImportListWidgetItem.h \
    gui/CrashReportDialog.h \
    gui/MultiQFileDialog.h \
    gui/MegaProxyStyle.h

RESOURCES += gui/Resources.qrc

INCLUDEPATH += $$PWD

debug {
    DEFINES += SHOW_LOGS
}

win32 {
    INCLUDEPATH += $$PWD/win
    FORMS    += gui/win/ActiveTransfer.ui \
        gui/win/InfoDialog.ui \
        gui/win/RecentFile.ui \
        gui/win/TransferProgressBar.ui \
        gui/win/UsageProgressBar.ui \
        gui/win/NodeSelector.ui \
        gui/win/FolderBinder.ui \
        gui/win/BindFolderDialog.ui \
        gui/win/UploadToMegaDialog.ui \
        gui/win/PasteMegaLinksDialog.ui \
        gui/win/ImportMegaLinksDialog.ui \
        gui/win/ImportListWidgetItem.ui \
        gui/win/CrashReportDialog.ui \
        gui/win/SetupWizard.ui \
        gui/win/SettingsDialog.ui
}

macx {
    INCLUDEPATH += $$PWD/macx
    FORMS    += gui/macx/ActiveTransfer.ui \
                gui/macx/InfoDialog.ui \
                gui/macx/RecentFile.ui \
                gui/macx/TransferProgressBar.ui \
                gui/macx/UsageProgressBar.ui \
                gui/macx/NodeSelector.ui \
                gui/macx/FolderBinder.ui \
                gui/macx/BindFolderDialog.ui \
                gui/macx/UploadToMegaDialog.ui \
                gui/macx/PasteMegaLinksDialog.ui \
                gui/macx/ImportMegaLinksDialog.ui \
                gui/macx/ImportListWidgetItem.ui \
                gui/macx/CrashReportDialog.ui \
                gui/macx/SetupWizard.ui \
                gui/macx/SettingsDialog.ui

    QT += macextras
    OBJECTIVE_SOURCES +=    gui/CocoaHelpButton.mm gui/MegaSystemTrayIcon.mm
    HEADERS += gui/CocoaHelpButton.h gui/MegaSystemTrayIcon.h
}

unix:!macx {
    INCLUDEPATH += $$PWD/linux
    FORMS    += gui/linux/ActiveTransfer.ui \
                gui/linux/InfoDialog.ui \
                gui/linux/RecentFile.ui \
                gui/linux/TransferProgressBar.ui \
                gui/linux/UsageProgressBar.ui \
                gui/linux/NodeSelector.ui \
                gui/linux/FolderBinder.ui \
                gui/linux/BindFolderDialog.ui \
                gui/linux/UploadToMegaDialog.ui \
                gui/linux/PasteMegaLinksDialog.ui \
                gui/linux/ImportMegaLinksDialog.ui \
                gui/linux/ImportListWidgetItem.ui \
                gui/linux/CrashReportDialog.ui \
                gui/linux/SetupWizard.ui \
                gui/linux/SettingsDialog.ui
}
