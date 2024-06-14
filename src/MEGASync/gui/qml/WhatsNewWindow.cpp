#include "WhatsNewWindow.h"

#include <QQmlEngine>

WhatsNewWindow::WhatsNewWindow(QObject *parent)
    : QMLComponent(parent)
{
    qmlRegisterModule("WhatsNewWidnow", 1, 0);
    qmlRegisterType<QmlDialog>("WhatsNewDialog", 1, 0, "WhatsNewDialog");
}

QUrl WhatsNewWindow::getQmlUrl()
{
    return QUrl(QString::fromUtf8("qrc:/whatsNew/WhatsNewDialog.qml"));
}

QString WhatsNewWindow::contextName()
{
    return QString::fromUtf8("whatsNewWindowAccess");
}
