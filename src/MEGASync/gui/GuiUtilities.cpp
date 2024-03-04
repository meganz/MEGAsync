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
#ifdef Q_OS_MAC
    msgInfo.text = title;
    msgInfo.informativeText = message;
#else
    msgInfo.title = title;
    msgInfo.text = message;
#endif

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
