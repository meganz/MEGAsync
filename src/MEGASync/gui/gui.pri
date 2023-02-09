QT       += network qml quick

QML_IMPORT_PATH = $$PWD/qml/MEGADesktopQML/imports

RESOURCES += $$PWD/qml/MEGADesktopQML/MEGADesktopQML.qrc

SOURCES += $$PWD/SettingsDialog.cpp \
    $$PWD/BalloonToolTip.cpp \
    $$PWD/BlurredShadowEffect.cpp \
    $$PWD/ButtonIconManager.cpp \
    $$PWD/LowDiskSpaceDialog.cpp \
    $$PWD/EventHelper.cpp \
    $$PWD/InfoDialog.cpp \
    $$PWD/MegaDelegateHoverManager.cpp \
    $$PWD/NotificationsSettings.cpp \
    $$PWD/OverQuotaDialog.cpp \
    $$PWD/QMLDialogWrapper.cpp \
    $$PWD/ScanningWidget.cpp \
    $$PWD/QtPositioningBugFixer.cpp \
    $$PWD/PasswordLineEdit.cpp \
    $$PWD/SetupWizard.cpp \
    $$PWD/UploadToMegaDialog.cpp \
    $$PWD/PasteMegaLinksDialog.cpp \
    $$PWD/ImportMegaLinksDialog.cpp \
    $$PWD/ImportListWidgetItem.cpp \
    $$PWD/CrashReportDialog.cpp \
    $$PWD/MultiQFileDialog.cpp \
    $$PWD/MegaProxyStyle.cpp \
    $$PWD/AccountDetailsDialog.cpp \
    $$PWD/DownloadFromMegaDialog.cpp \
    $$PWD/ChangeLogDialog.cpp \
    $$PWD/GuestWidget.cpp \
    $$PWD/StreamingFromMegaDialog.cpp \
    $$PWD/MegaProgressCustomDialog.cpp \
    $$PWD/UpgradeDialog.cpp \
    $$PWD/PlanWidget.cpp \
    $$PWD/InfoWizard.cpp \
    $$PWD/QMegaMessageBox.cpp \
    $$PWD/AvatarWidget.cpp \
    $$PWD/MenuItemAction.cpp \
    $$PWD/AddExclusionDialog.cpp \
    $$PWD/StatusInfo.cpp \
    $$PWD/ChangePassword.cpp \
    $$PWD/PSAwidget.cpp \
    $$PWD/ElidedLabel.cpp \
    $$PWD/UpgradeOverStorage.cpp \
    $$PWD/Login2FA.cpp \
    $$PWD/QRWidget.cpp \
    $$PWD/CircularUsageProgressBar.cpp \
    $$PWD/HighDpiResize.cpp \
    $$PWD/AlertItem.cpp \
    $$PWD/QAlertsModel.cpp \
    $$PWD/MegaAlertDelegate.cpp \
    $$PWD/QFilterAlertsModel.cpp \
    $$PWD/FilterAlertWidget.cpp \
    $$PWD/AlertFilterType.cpp \
    $$PWD/BugReportDialog.cpp \
    $$PWD/VerifyLockMessage.cpp \
    $$PWD/MegaInfoMessage.cpp \
    $$PWD/WaitingSpinnerWidget.cpp \
    $$PWD/ProxySettings.cpp \
    $$PWD/BandwidthSettings.cpp \
    $$PWD/SwitchButton.cpp \
    $$PWD/GuiUtilities.cpp \
    $$PWD/CancelConfirmWidget.cpp \
    $$PWD/NodeNameSetterDialog/NodeNameSetterDialog.cpp \
    $$PWD/NodeNameSetterDialog/NewFolderDialog.cpp \
    $$PWD/NodeNameSetterDialog/RenameNodeDialog.cpp \   
    $$PWD/node_selector/model/NodeSelectorDelegates.cpp \
    $$PWD/node_selector/model/NodeSelectorProxyModel.cpp \
    $$PWD/node_selector/model/NodeSelectorModel.cpp \
    $$PWD/node_selector/model/NodeSelectorModelSpecialised.cpp \
    $$PWD/node_selector/model/NodeSelectorModelItem.cpp \
    $$PWD/node_selector/gui/NodeSelectorTreeView.cpp \
    $$PWD/node_selector/gui/NodeSelectorTreeViewWidget.cpp \
    $$PWD/node_selector/gui/NodeSelectorTreeViewWidgetSpecializations.cpp \
    $$PWD/node_selector/gui/NodeSelector.cpp \
    $$PWD/node_selector/gui/NodeSelectorLoadingDelegate.cpp \
    $$PWD/node_selector/gui/SearchLineEdit.cpp \
    $$PWD/node_selector/gui/NodeSelectorSpecializations.cpp


HEADERS  += $$PWD/SettingsDialog.h \
    $$PWD/AutoResizeStackedWidget.h \
    $$PWD/BalloonToolTip.h \
    $$PWD/BlurredShadowEffect.h \
    $$PWD/ButtonIconManager.h \
    $$PWD/DialogGeometryRetainer.h \
    $$PWD/LowDiskSpaceDialog.h \
    $$PWD/EventHelper.h \
    $$PWD/InfoDialog.h \
    $$PWD/MegaDelegateHoverManager.h \
    $$PWD/MegaNodeNames.h \
    $$PWD/NotificationsSettings.h \
    $$PWD/OverQuotaDialog.h \
    $$PWD/QMLDialogWrapper.h \
    $$PWD/ScanningWidget.h \
    $$PWD/QtPositioningBugFixer.h \
    $$PWD/PasswordLineEdit.h \
    $$PWD/SetupWizard.h \
    $$PWD/UploadToMegaDialog.h \
    $$PWD/PasteMegaLinksDialog.h \
    $$PWD/ImportMegaLinksDialog.h \
    $$PWD/ImportListWidgetItem.h \
    $$PWD/CrashReportDialog.h \
    $$PWD/MultiQFileDialog.h \
    $$PWD/MegaProxyStyle.h \
    $$PWD/AccountDetailsDialog.h \
    $$PWD/DownloadFromMegaDialog.h \
    $$PWD/ChangeLogDialog.h \
    $$PWD/GuestWidget.h \
    $$PWD/StreamingFromMegaDialog.h \
    $$PWD/MegaProgressCustomDialog.h \
    $$PWD/UpgradeDialog.h \
    $$PWD/PlanWidget.h \
    $$PWD/InfoWizard.h \
    $$PWD/QMegaMessageBox.h \
    $$PWD/AvatarWidget.h \
    $$PWD/MenuItemAction.h \
    $$PWD/AddExclusionDialog.h \
    $$PWD/StatusInfo.h \
    $$PWD/PSAwidget.h \
    $$PWD/ElidedLabel.h \
    $$PWD/UpgradeOverStorage.h \
    $$PWD/ChangePassword.h \
    $$PWD/Login2FA.h \
    $$PWD/QRWidget.h \
    $$PWD/CircularUsageProgressBar.h \
    $$PWD/HighDpiResize.h \
    $$PWD/AlertItem.h \
    $$PWD/QAlertsModel.h \
    $$PWD/MegaAlertDelegate.h \
    $$PWD/QFilterAlertsModel.h \
    $$PWD/FilterAlertWidget.h \
    $$PWD/AlertFilterType.h \
    $$PWD/BugReportDialog.h \
    $$PWD/VerifyLockMessage.h \
    $$PWD/ViewLoadingScene.h \
    $$PWD/MegaInfoMessage.h \
    $$PWD/WaitingSpinnerWidget.h \
    $$PWD/ProxySettings.h \
    $$PWD/BandwidthSettings.h \
    $$PWD/SwitchButton.h \
    $$PWD/GuiUtilities.h \
    $$PWD/CancelConfirmWidget.h \
    $$PWD/NodeNameSetterDialog/NodeNameSetterDialog.h \
    $$PWD/NodeNameSetterDialog/NewFolderDialog.h \
    $$PWD/NodeNameSetterDialog/RenameNodeDialog.h \
    $$PWD/node_selector/model/NodeSelectorDelegates.h \
    $$PWD/node_selector/model/NodeSelectorProxyModel.h \
    $$PWD/node_selector/model/NodeSelectorModel.h \
    $$PWD/node_selector/model/NodeSelectorModelSpecialised.h \
    $$PWD/node_selector/model/NodeSelectorModelItem.h \
    $$PWD/node_selector/gui/NodeSelectorTreeView.h \
    $$PWD/node_selector/gui/NodeSelectorTreeViewWidget.h \
    $$PWD/node_selector/gui/NodeSelectorTreeViewWidgetSpecializations.h \
    $$PWD/node_selector/gui/NodeSelector.h \
    $$PWD/node_selector/gui/NodeSelectorLoadingDelegate.h \
    $$PWD/node_selector/gui/SearchLineEdit.h \
    $$PWD/node_selector/gui/NodeSelectorSpecializations.h


INCLUDEPATH += $$PWD

debug {
    DEFINES += SHOW_LOGS
}

win32 {
    RESOURCES += $$PWD/Resources_win.qrc
    INCLUDEPATH += $$PWD/win
    FORMS    += $$PWD/win/InfoDialog.ui \
                $$PWD/win/UploadToMegaDialog.ui \
                $$PWD/win/PasteMegaLinksDialog.ui \
                $$PWD/win/ImportMegaLinksDialog.ui \
                $$PWD/win/ImportListWidgetItem.ui \
                $$PWD/win/CrashReportDialog.ui \
                $$PWD/win/SetupWizard.ui \
                $$PWD/win/SettingsDialog.ui \
                $$PWD/win/AccountDetailsDialog.ui \
                $$PWD/win/DownloadFromMegaDialog.ui \
                $$PWD/win/ChangeLogDialog.ui \
                $$PWD/win/GuestWidget.ui \
                $$PWD/win/StreamingFromMegaDialog.ui \
                $$PWD/win/MegaProgressCustomDialog.ui \
                $$PWD/win/PlanWidget.ui \
                $$PWD/win/UpgradeDialog.ui \
                $$PWD/win/InfoWizard.ui \
                $$PWD/win/AddExclusionDialog.ui \
                $$PWD/win/StatusInfo.ui \
                $$PWD/win/PSAwidget.ui \
                $$PWD/win/UpgradeOverStorage.ui \
                $$PWD/win/ChangePassword.ui \
                $$PWD/win/Login2FA.ui \
                $$PWD/win/AlertItem.ui \
                $$PWD/win/FilterAlertWidget.ui \
                $$PWD/win/AlertFilterType.ui \
                $$PWD/win/BugReportDialog.ui \
                $$PWD/win/LockedPopOver.ui \
                $$PWD/win/VerifyLockMessage.ui \
                $$PWD/win/MegaInfoMessage.ui \
                $$PWD/win/OverQuotaDialog.ui \
                $$PWD/win/ProxySettings.ui \
                $$PWD/win/BandwidthSettings.ui \
                $$PWD/win/ScanningWidget.ui \
                $$PWD/win/CancelConfirmWidget.ui \
                $$PWD/win/NodeNameSetterDialog.ui \
                $$PWD/win/NotificationsSettings.ui \
                $$PWD/win/LowDiskSpaceDialog.ui \
                $$PWD/node_selector/gui/win/NodeSelectorTreeViewWidget.ui \
                $$PWD/node_selector/gui/win/NodeSelectorLoadingDelegate.ui \
                $$PWD/node_selector/gui/win/NodeSelector.ui \
                $$PWD/node_selector/gui/win/SearchLineEdit.ui
}

macx {
    RESOURCES += $$PWD/Resources_macx.qrc
    INCLUDEPATH += $$PWD/macx
    FORMS    += $$PWD/macx/InfoDialog.ui \
                $$PWD/macx/UploadToMegaDialog.ui \
                $$PWD/macx/PasteMegaLinksDialog.ui \
                $$PWD/macx/ImportMegaLinksDialog.ui \
                $$PWD/macx/ImportListWidgetItem.ui \
                $$PWD/macx/CrashReportDialog.ui \
                $$PWD/macx/SetupWizard.ui \
                $$PWD/macx/SettingsDialog.ui \
                $$PWD/macx/AccountDetailsDialog.ui \
                $$PWD/macx/DownloadFromMegaDialog.ui \
                $$PWD/macx/ChangeLogDialog.ui \
                $$PWD/macx/GuestWidget.ui \
                $$PWD/macx/StreamingFromMegaDialog.ui \
                $$PWD/macx/PermissionsDialog.ui \
                $$PWD/macx/PermissionsWidget.ui \
                $$PWD/macx/MegaProgressCustomDialog.ui \
                $$PWD/macx/PlanWidget.ui \
                $$PWD/macx/UpgradeDialog.ui \
                $$PWD/macx/InfoWizard.ui \
                $$PWD/macx/AddExclusionDialog.ui \
                $$PWD/macx/StatusInfo.ui \
                $$PWD/macx/PSAwidget.ui \
                $$PWD/macx/UpgradeOverStorage.ui \
                $$PWD/macx/ChangePassword.ui \
                $$PWD/macx/Login2FA.ui \
                $$PWD/macx/AlertItem.ui \
                $$PWD/macx/FilterAlertWidget.ui \
                $$PWD/macx/AlertFilterType.ui \
                $$PWD/macx/BugReportDialog.ui \
                $$PWD/macx/LockedPopOver.ui \
                $$PWD/macx/VerifyLockMessage.ui \
                $$PWD/macx/MegaInfoMessage.ui \
                $$PWD/macx/OverQuotaDialog.ui \
                $$PWD/macx/ProxySettings.ui \
                $$PWD/macx/BandwidthSettings.ui \
                $$PWD/macx/ScanningWidget.ui \
                $$PWD/macx/CancelConfirmWidget.ui \
                $$PWD/macx/NodeNameSetterDialog.ui \
                $$PWD/macx/NotificationsSettings.ui \
                $$PWD/macx/LowDiskSpaceDialog.ui \
                $$PWD/node_selector/gui/macx/NodeSelectorTreeViewWidget.ui \
                $$PWD/node_selector/gui/macx/NodeSelectorLoadingDelegate.ui \
                $$PWD/node_selector/gui/macx/NodeSelector.ui \
                $$PWD/node_selector/gui/macx/SearchLineEdit.ui

    #Asset catalog need to load SF symbol images of toolbar items for custom NSToolbar
    QMAKE_ASSET_CATALOGS += $$PWD/images/Images.xcassets

    QT += macextras

    OBJECTIVE_SOURCES +=    $$PWD/CocoaHelpButton.mm \
                            $$PWD/CocoaSwitchButton.mm \
                            $$PWD/MegaSystemTrayIcon.mm \
                            $$PWD/QMacSpinningProgressIndicator.mm \
                            $$PWD/QSegmentedControl.mm

    HEADERS += $$PWD/CocoaHelpButton.h $$PWD/CocoaSwitchButton.h $$PWD/MegaSystemTrayIcon.h $$PWD/QSegmentedControl.h

    HEADERS += $$PWD/PermissionsDialog.h \
               $$PWD/PermissionsWidget.h \
               $$PWD/QMacSpinningProgressIndicator.h
    SOURCES += $$PWD/PermissionsDialog.cpp \
               $$PWD/PermissionsWidget.cpp
}
 else {
    HEADERS += $$PWD/LockedPopOver.h
    SOURCES += $$PWD/LockedPopOver.cpp
}

unix:!macx {
    RESOURCES += $$PWD/Resources_linux.qrc
    INCLUDEPATH += $$PWD/linux
    FORMS    += $$PWD/linux/InfoDialog.ui \
                $$PWD/linux/UploadToMegaDialog.ui \
                $$PWD/linux/PasteMegaLinksDialog.ui \
                $$PWD/linux/ImportMegaLinksDialog.ui \
                $$PWD/linux/ImportListWidgetItem.ui \
                $$PWD/linux/CrashReportDialog.ui \
                $$PWD/linux/SetupWizard.ui \
                $$PWD/linux/SettingsDialog.ui \
                $$PWD/linux/AccountDetailsDialog.ui \
                $$PWD/linux/DownloadFromMegaDialog.ui \
                $$PWD/linux/ChangeLogDialog.ui \
                $$PWD/linux/GuestWidget.ui \
                $$PWD/linux/StreamingFromMegaDialog.ui \
                $$PWD/linux/PermissionsDialog.ui \
                $$PWD/linux/PermissionsWidget.ui \
                $$PWD/linux/MegaProgressCustomDialog.ui \
                $$PWD/linux/PlanWidget.ui \
                $$PWD/linux/UpgradeDialog.ui \
                $$PWD/linux/InfoWizard.ui \
                $$PWD/linux/AddExclusionDialog.ui \
                $$PWD/linux/StatusInfo.ui \
                $$PWD/linux/PSAwidget.ui \
                $$PWD/linux/UpgradeOverStorage.ui \
                $$PWD/linux/ChangePassword.ui \
                $$PWD/linux/Login2FA.ui \
                $$PWD/linux/AlertItem.ui \
                $$PWD/linux/FilterAlertWidget.ui \
                $$PWD/linux/AlertFilterType.ui \
                $$PWD/linux/BugReportDialog.ui \
                $$PWD/linux/LockedPopOver.ui \
                $$PWD/linux/VerifyLockMessage.ui \
                $$PWD/linux/MegaInfoMessage.ui \
                $$PWD/linux/OverQuotaDialog.ui \
                $$PWD/linux/ProxySettings.ui \
                $$PWD/linux/BandwidthSettings.ui \
                $$PWD/linux/CancelConfirmWidget.ui \
                $$PWD/linux/ScanningWidget.ui \
                $$PWD/linux/NodeNameSetterDialog.ui \
                $$PWD/linux/NotificationsSettings.ui \
                $$PWD/linux/LowDiskSpaceDialog.ui \
                $$PWD/node_selector/gui/linux/NodeSelectorTreeViewWidget.ui \
                $$PWD/node_selector/gui/linux/NodeSelectorLoadingDelegate.ui \
                $$PWD/node_selector/gui/linux/NodeSelector.ui \
                $$PWD/node_selector/gui/linux/SearchLineEdit.ui

    HEADERS += $$PWD/PermissionsDialog.h \
               $$PWD/PermissionsWidget.h
    SOURCES += $$PWD/PermissionsDialog.cpp \
               $$PWD/PermissionsWidget.cpp
}

