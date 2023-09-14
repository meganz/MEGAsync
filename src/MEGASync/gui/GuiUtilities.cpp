#include "gui/GuiUtilities.h"

#include <QApplication>
#include <QPointer>
#include <QUrl>

#include "QMegaMessageBox.h"
#include "TextDecorator.h"
#include "Utilities.h"

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

void GuiUtilities::connectAddSyncDefaultHandler(SyncController *controller, const int accountType)
{
    auto defaultHandler = [accountType](const int errorCode, const int syncErrorCode,
                                        const QString errorMsg, QString localPath)
    {
        if (errorCode != MegaError::API_OK)
        {
            showAddSyncError(accountType, syncErrorCode, errorMsg, localPath);
        }
    };
    SyncController::connect(controller, &SyncController::syncAddStatus, controller, defaultHandler);
}

void GuiUtilities::showAddSyncError(const int accountType, const int syncErrorCode,
                      const QString& errorMessage, const QString& localPath)
{
    const QString title = QCoreApplication::translate("GuiUtilities", "Error adding sync");

    if (accountType == MegaAccountDetails::ACCOUNT_TYPE_PRO_FLEXI && syncErrorCode == MegaSync::ACCOUNT_EXPIRED)
    {
        QString message = QCoreApplication::translate("GuiUtilities", "%1 can't be added as your Pro Flexi account has been deactivated due to payment failure "
                     "or you've cancelled your subscription. To continue, make a payment and reactivate your subscription.").arg(localPath);
        showPayReactivateOrDismiss(title, message);
    }
    else
    {
        QMegaMessageBox::MessageBoxInfo msgInfo;
        msgInfo.parent = nullptr;
        msgInfo.title = title;
        msgInfo.text = decoratedWithSupportUrl(errorMessage);
        QMegaMessageBox::warning(msgInfo);
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

    QMegaMessageBox::MessageBoxInfo msgInfo;
    msgInfo.parent = nullptr;
    msgInfo.text = title;
    msgInfo.informativeText = message;

    msgInfo.buttons = QMessageBox::Yes | QMessageBox::No;
    msgInfo.buttonsText.insert(QMessageBox::Yes, payButtonLabel);
    msgInfo.buttonsText.insert(QMessageBox::No, dismissLabel);
    msgInfo.defaultButton = QMessageBox::Yes;

    msgInfo.finishFunc = [](QPointer<QMessageBox> msg)
    {
        if(msg->result() == QMessageBox::Yes)
        {
            QString url = QString::fromUtf8("mega://#repay");
            Utilities::getPROurlWithParameters(url);
            Utilities::openUrl(QUrl(url));
        }
    };

    QMegaMessageBox::warning(msgInfo);
}

QString GuiUtilities::decoratedWithSupportUrl(const QString &message)
{
    const QString supportUrl = QString::fromUtf8("https://mega.nz/contact");

    Text::Link link(supportUrl);
    Text::Decorator dec(&link);
    QString decoratedMessage = message;
    dec.process(decoratedMessage);
    return decoratedMessage;
}
