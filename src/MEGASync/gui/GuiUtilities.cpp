#include "GuiUtilities.h"

#include "MessageDialogOpener.h"
#include "ServiceUrls.h"
#include "Utilities.h"

#include <QApplication>
#include <QPointer>
#include <QUrl>

using namespace mega;

void GuiUtilities::updateDataRequestProgressBar(QProgressBar* progressBar, MegaRequest *request)
{
    if (request->getType() == MegaRequest::TYPE_FETCH_NODES)
    {
        if (request->getTotalBytes() > 0)
        {
            double total = static_cast<double>(request->getTotalBytes());
            double part = static_cast<double>(request->getTransferredBytes());
            progressBar->setMaximum(100);
            progressBar->setValue(static_cast<int>(100.0 * part / total));
        }
    }
}

void GuiUtilities::showPayNowOrDismiss(const QString &title, const QString &message)
{
    const QString payLabel = QCoreApplication::translate("GuiUtilities", "Pay Now");
    showPayOrDismiss(title, message, payLabel);
}

void GuiUtilities::showPayReactivateOrDismiss(const QString &title, const QString &message)
{
    const QString payLabel = QCoreApplication::translate("GuiUtilities", "Pay and reactivate");
    showPayOrDismiss(title, message, payLabel);
}

void GuiUtilities::showPayOrDismiss(const QString &title, const QString &message, const QString &payButtonLabel)
{
    const QString dismissLabel = QCoreApplication::translate("GuiUtilities", "Dismiss");

    MessageDialogInfo msgInfo;
    msgInfo.parent = nullptr;
    msgInfo.titleText = title;
    msgInfo.descriptionText = message;
    msgInfo.buttons = QMessageBox::Yes | QMessageBox::No;
    msgInfo.buttonsText.insert(QMessageBox::Yes, payButtonLabel);
    msgInfo.buttonsText.insert(QMessageBox::No, dismissLabel);
    msgInfo.defaultButton = QMessageBox::Yes;
    msgInfo.textFormat = Qt::TextFormat::RichText;
    msgInfo.finishFunc = [](QPointer<MessageDialogResult> msg)
    {
        if(msg->result() == QMessageBox::Yes)
        {
            Utilities::openUrl(ServiceUrls::getRepayUrl());
        }
    };

    MessageDialogOpener::warning(msgInfo);
}
