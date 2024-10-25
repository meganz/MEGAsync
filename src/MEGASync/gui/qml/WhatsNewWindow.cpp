#include "WhatsNewWindow.h"

#include "UpdatesModel.h"

#include <QQmlEngine>

WhatsNewWindow::WhatsNewWindow(QObject* parent):
    QMLComponent(parent),
    mController(std::make_shared<WhatsNewController>()),
    mModel(std::make_shared<UpdatesModel>(mController))
{}

QUrl WhatsNewWindow::getQmlUrl()
{
    return QUrl(QString::fromUtf8("qrc:/whatsNew/WhatsNewDialog.qml"));
}

QString WhatsNewWindow::contextName()
{
    return QString::fromUtf8("whatsNewWindowAccess");
}

QString WhatsNewWindow::acceptButtonText()
{
    if (!mController)
    {
        return {};
    }
    return mController->acceptButtonText();
}

void WhatsNewWindow::acceptButtonClicked()
{
    if (!mController)
    {
        return;
    }
    mController->acceptButtonClicked();
}
