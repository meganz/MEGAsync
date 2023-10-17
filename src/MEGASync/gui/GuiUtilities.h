#ifndef GUIUTILITIES_H
#define GUIUTILITIES_H

#include <QProgressBar>
#include "megaapi.h"
#include "syncs/control/SyncController.h"

class GuiUtilities
{
public:
    static void updateDataRequestProgressBar(QProgressBar* progressBar, mega::MegaRequest *request);

    static void connectAddSyncDefaultHandler(SyncController* controller, const int accountType);

    static void showPayNowOrDismiss(const QString &title, const QString &message);
    static void showPayReactivateOrDismiss(const QString &title, const QString &message);

    static QString decoratedWithSupportUrl(const QString& message);

private:
    static void showAddSyncError(const int accountType, const int syncErrorCode,
                          const QString& errorMessage, const QString& localPath);

    static void showPayOrDismiss(const QString &title, const QString &message, const QString& payButtonLabel);
};

#endif // GUIUTILITIES_H
