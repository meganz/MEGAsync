#ifndef GUIUTILITIES_H
#define GUIUTILITIES_H

#include <QProgressBar>
#include "megaapi.h"
#include "syncs/control/SyncController.h"

class GuiUtilities
{
public:
    static void updateDataRequestProgressBar(QProgressBar* progressBar, mega::MegaRequest *request);

    static void showPayNowOrDismiss(const QString &title, const QString &message);
    static void showPayReactivateOrDismiss(const QString &title, const QString &message);

private:
    static void showPayOrDismiss(const QString &title, const QString &message, const QString& payButtonLabel);
};

#endif // GUIUTILITIES_H
