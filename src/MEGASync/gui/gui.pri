QT       += network

SOURCES += $$PWD/SettingsDialog.cpp \
    $$PWD/ActiveTransfer.cpp \
    $$PWD/InfoDialog.cpp \
    $$PWD/RecentFile.cpp \
    $$PWD/TransferProgressBar.cpp \
    $$PWD/UsageProgressBar.cpp \
    $$PWD/SetupWizard.cpp \
    $$PWD/NodeSelector.cpp \
    $$PWD/FolderBinder.cpp \
    $$PWD/BindFolderDialog.cpp \
    $$PWD/UploadToMegaDialog.cpp \
    $$PWD/PasteMegaLinksDialog.cpp \
    $$PWD/ImportMegaLinksDialog.cpp \
    $$PWD/ImportListWidgetItem.cpp \
    $$PWD/CrashReportDialog.cpp \
    $$PWD/MultiQFileDialog.cpp \
    $$PWD/MegaProxyStyle.cpp \
    $$PWD/AccountDetailsDialog.cpp \
    $$PWD/DownloadFromMegaDialog.cpp \
    $$PWD/SizeLimitDialog.cpp \
    $$PWD/UsageWidget.cpp \
    $$PWD/MessageBox.cpp \
    $$PWD/InfoOverQuotaDialog.cpp \
    $$PWD/QMegaModel.cpp \
    $$PWD/MegaItem.cpp \
    $$PWD/ChangeLogDialog.cpp \
    $$PWD/GuestWidget.cpp \
    $$PWD/StreamingFromMegaDialog.cpp \
    $$PWD/ConfirmSSLexception.cpp \
    $$PWD/UpgradeDialog.cpp \
    $$PWD/PlanWidget.cpp \
    $$PWD/InfoWizard.cpp \
    $$PWD/TransferItem.cpp \
    $$PWD/TransferManager.cpp \
    $$PWD/TransfersWidget.cpp \
    $$PWD/QTransfersModel.cpp \
    $$PWD/MegaTransferDelegate.cpp \
    $$PWD/MegaTransferView.cpp \
    $$PWD/QMegaMessageBox.cpp \
    $$PWD/TransferMenuItemAction.cpp

HEADERS  += $$PWD/SettingsDialog.h \
    $$PWD/ActiveTransfer.h \
    $$PWD/InfoDialog.h \
    $$PWD/RecentFile.h \
    $$PWD/TransferProgressBar.h \
    $$PWD/UsageProgressBar.h \
    $$PWD/SetupWizard.h \
    $$PWD/NodeSelector.h \
    $$PWD/FolderBinder.h \
    $$PWD/BindFolderDialog.h \
    $$PWD/UploadToMegaDialog.h \
    $$PWD/PasteMegaLinksDialog.h \
    $$PWD/ImportMegaLinksDialog.h \
    $$PWD/ImportListWidgetItem.h \
    $$PWD/CrashReportDialog.h \
    $$PWD/MultiQFileDialog.h \
    $$PWD/MegaProxyStyle.h \
    $$PWD/AccountDetailsDialog.h \
    $$PWD/DownloadFromMegaDialog.h \
    $$PWD/SizeLimitDialog.h \
    $$PWD/UsageWidget.h \
    $$PWD/MessageBox.h \
    $$PWD/InfoOverQuotaDialog.h \
    $$PWD/QMegaModel.h \
    $$PWD/MegaItem.h \
    $$PWD/ChangeLogDialog.h \
    $$PWD/GuestWidget.h \
    $$PWD/StreamingFromMegaDialog.h \
    $$PWD/ConfirmSSLexception.h \
    $$PWD/UpgradeDialog.h \
    $$PWD/PlanWidget.h \
    $$PWD/InfoWizard.h \
    $$PWD/TransferItem.h \
    $$PWD/TransferManager.h \
    $$PWD/TransfersWidget.h \
    $$PWD/QTransfersModel.h \
    $$PWD/MegaTransferDelegate.h \
    $$PWD/MegaTransferView.h \
    $$PWD/QMegaMessageBox.h \
    $$PWD/TransferMenuItemAction.h

INCLUDEPATH += $$PWD

debug {
    DEFINES += SHOW_LOGS
}

win32 {
    RESOURCES += $$PWD/Resources_win.qrc
    INCLUDEPATH += $$PWD/win
    FORMS    += $$PWD/win/ActiveTransfer.ui \
                $$PWD/win/InfoDialog.ui \
                $$PWD/win/RecentFile.ui \
                $$PWD/win/TransferProgressBar.ui \
                $$PWD/win/UsageProgressBar.ui \
                $$PWD/win/NodeSelector.ui \
                $$PWD/win/FolderBinder.ui \
                $$PWD/win/BindFolderDialog.ui \
                $$PWD/win/UploadToMegaDialog.ui \
                $$PWD/win/PasteMegaLinksDialog.ui \
                $$PWD/win/ImportMegaLinksDialog.ui \
                $$PWD/win/ImportListWidgetItem.ui \
                $$PWD/win/CrashReportDialog.ui \
                $$PWD/win/SetupWizard.ui \
                $$PWD/win/SettingsDialog.ui \
                $$PWD/win/AccountDetailsDialog.ui \
                $$PWD/win/DownloadFromMegaDialog.ui \
                $$PWD/win/SizeLimitDialog.ui \
                $$PWD/win/MessageBox.ui \
                $$PWD/win/InfoOverQuotaDialog.ui \
                $$PWD/win/ChangeLogDialog.ui \
                $$PWD/win/GuestWidget.ui \
                $$PWD/win/StreamingFromMegaDialog.ui \
                $$PWD/win/ConfirmSSLexception.ui \
                $$PWD/win/PlanWidget.ui \
                $$PWD/win/UpgradeDialog.ui \
                $$PWD/win/InfoWizard.ui \
                $$PWD/win/TransferItem.ui \
                $$PWD/win/TransferManager.ui \
                $$PWD/win/TransfersWidget.ui \
}

macx {
    RESOURCES += $$PWD/Resources_macx.qrc
    INCLUDEPATH += $$PWD/macx
    FORMS    += $$PWD/macx/ActiveTransfer.ui \
                $$PWD/macx/InfoDialog.ui \
                $$PWD/macx/RecentFile.ui \
                $$PWD/macx/TransferProgressBar.ui \
                $$PWD/macx/UsageProgressBar.ui \
                $$PWD/macx/NodeSelector.ui \
                $$PWD/macx/FolderBinder.ui \
                $$PWD/macx/BindFolderDialog.ui \
                $$PWD/macx/UploadToMegaDialog.ui \
                $$PWD/macx/PasteMegaLinksDialog.ui \
                $$PWD/macx/ImportMegaLinksDialog.ui \
                $$PWD/macx/ImportListWidgetItem.ui \
                $$PWD/macx/CrashReportDialog.ui \
                $$PWD/macx/SetupWizard.ui \
                $$PWD/macx/SettingsDialog.ui \
                $$PWD/macx/AccountDetailsDialog.ui \
                $$PWD/macx/DownloadFromMegaDialog.ui \
                $$PWD/macx/SizeLimitDialog.ui \
                $$PWD/macx/MessageBox.ui \
                $$PWD/macx/InfoOverQuotaDialog.ui \
                $$PWD/macx/ChangeLogDialog.ui \
                $$PWD/macx/GuestWidget.ui \
                $$PWD/macx/StreamingFromMegaDialog.ui \
                $$PWD/macx/PermissionsDialog.ui \
                $$PWD/macx/PermissionsWidget.ui \
                $$PWD/macx/ConfirmSSLexception.ui \
                $$PWD/macx/PlanWidget.ui \
                $$PWD/macx/UpgradeDialog.ui \
                $$PWD/macx/InfoWizard.ui \
                $$PWD/macx/TransferItem.ui \
                $$PWD/macx/TransferManager.ui \
                $$PWD/macx/TransfersWidget.ui \

    QT += macextras
    OBJECTIVE_SOURCES +=    gui/CocoaHelpButton.mm gui/MegaSystemTrayIcon.mm
    HEADERS += gui/CocoaHelpButton.h gui/MegaSystemTrayIcon.h

    HEADERS += $$PWD/PermissionsDialog.h \
               $$PWD/PermissionsWidget.h
    SOURCES += $$PWD/PermissionsDialog.cpp \
               $$PWD/PermissionsWidget.cpp
}

unix:!macx {
    RESOURCES += $$PWD/Resources_linux.qrc
    INCLUDEPATH += $$PWD/linux
    FORMS    += $$PWD/linux/ActiveTransfer.ui \
                $$PWD/linux/InfoDialog.ui \
                $$PWD/linux/RecentFile.ui \
                $$PWD/linux/TransferProgressBar.ui \
                $$PWD/linux/UsageProgressBar.ui \
                $$PWD/linux/NodeSelector.ui \
                $$PWD/linux/FolderBinder.ui \
                $$PWD/linux/BindFolderDialog.ui \
                $$PWD/linux/UploadToMegaDialog.ui \
                $$PWD/linux/PasteMegaLinksDialog.ui \
                $$PWD/linux/ImportMegaLinksDialog.ui \
                $$PWD/linux/ImportListWidgetItem.ui \
                $$PWD/linux/CrashReportDialog.ui \
                $$PWD/linux/SetupWizard.ui \
                $$PWD/linux/SettingsDialog.ui \
                $$PWD/linux/AccountDetailsDialog.ui \
                $$PWD/linux/DownloadFromMegaDialog.ui \
                $$PWD/linux/SizeLimitDialog.ui \
                $$PWD/linux/MessageBox.ui\
                $$PWD/linux/InfoOverQuotaDialog.ui \
                $$PWD/linux/ChangeLogDialog.ui \
                $$PWD/linux/GuestWidget.ui \
                $$PWD/linux/StreamingFromMegaDialog.ui \
                $$PWD/linux/PermissionsDialog.ui \
                $$PWD/linux/PermissionsWidget.ui \
                $$PWD/linux/ConfirmSSLexception.ui \
                $$PWD/linux/PlanWidget.ui \
                $$PWD/linux/UpgradeDialog.ui \
                $$PWD/linux/InfoWizard.ui \
                $$PWD/linux/TransferItem.ui \
                $$PWD/linux/TransferManager.ui \
                $$PWD/linux/TransfersWidget.ui

    HEADERS += $$PWD/PermissionsDialog.h \
               $$PWD/PermissionsWidget.h
    SOURCES += $$PWD/PermissionsDialog.cpp \
               $$PWD/PermissionsWidget.cpp
}
