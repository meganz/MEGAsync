
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
    gui/BindFolderDialog.cpp

HEADERS  += gui/SettingsDialog.h \
    gui/ActiveTransfer.h \
    gui/InfoDialog.h \
    gui/RecentFile.h \
    gui/TransferProgressBar.h \
    gui/UsageProgressBar.h \
    gui/SetupWizard.h \
    gui/NodeSelector.h \
    gui/FolderBinder.h \
    gui/BindFolderDialog.h


FORMS    += gui/SettingsDialog.ui \
    gui/ActiveTransfer.ui \
    gui/InfoDialog.ui \
    gui/RecentFile.ui \
    gui/TransferProgressBar.ui \
    gui/UsageProgressBar.ui \
    gui/SetupWizard.ui \
    gui/NodeSelector.ui \
    gui/FolderBinder.ui \
    gui/BindFolderDialog.ui

RESOURCES += gui/Resources.qrc

