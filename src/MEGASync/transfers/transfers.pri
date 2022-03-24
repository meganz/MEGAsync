DEPENDPATH += $$PWD
INCLUDEPATH += $$PWD
INCLUDEPATH += $$PWD/model
INCLUDEPATH += $$PWD/gui

SOURCES += $$PWD/model/TransfersModel.cpp \
           $$PWD/model/InfoDialogTransfersProxyModel.cpp \
           $$PWD/model/TransfersManagerSortFilterProxyModel.cpp \
           $$PWD/gui/InfoDialogTransferDelegateWidget.cpp \
           $$PWD/gui/InfoDialogTransfersWidget.cpp \
           $$PWD/gui/MegaTransferDelegate.cpp  \
           $$PWD/gui/MegaTransferView.cpp \
           $$PWD/gui/TransferBaseDelegateWidget.cpp \
           $$PWD/gui/TransferItem.cpp \
           $$PWD/gui/TransferManager.cpp \
           $$PWD/gui/TransferManagerDelegateWidget.cpp \
           $$PWD/gui/InfoDialogTransfersWidget.cpp  \
           $$PWD/gui/TransferManagerLoadingItem.cpp \
           $$PWD/gui/TransfersStateInfoWidget.cpp \
           $$PWD/gui/TransfersStatusWidget.cpp \
           $$PWD/gui/TransfersSummaryWidget.cpp \
           $$PWD/gui/TransfersWidget.cpp

HEADERS += $$PWD/model/InfoDialogTransfersProxyModel.h \
           $$PWD/model/TransfersManagerSortFilterProxyModel.h \
           $$PWD/model/TransfersSortFilterProxyBaseModel.h \
           $$PWD/model/TransfersModel.h \
           $$PWD/gui/InfoDialogTransferDelegateWidget.h \
           $$PWD/gui/InfoDialogTransfersWidget.h \
           $$PWD/gui/MegaTransferDelegate.h  \
           $$PWD/gui/MegaTransferView.h \
           $$PWD/gui/TransferBaseDelegateWidget.h \
           $$PWD/gui/TransferItem.h \
           $$PWD/gui/TransferManager.h \
           $$PWD/gui/TransferManagerDelegateWidget.h \
           $$PWD/gui/InfoDialogTransfersWidget.h  \
           $$PWD/gui/TransferManagerLoadingItem.h \
           $$PWD/gui/TransfersStateInfoWidget.h \
           $$PWD/gui/TransfersStatusWidget.h \
           $$PWD/gui/TransfersSummaryWidget.h \
           $$PWD/gui/TransfersWidget.h
