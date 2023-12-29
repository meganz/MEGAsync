#include "QmlDialog.h"

#include <QEvent>

QmlDialog::QmlDialog(QWindow *parent)
    : QQuickWindow(parent)
{
    setFlags(flags() | Qt::Dialog);
    setIcon(QIcon(QString::fromUtf8("://images/app_ico.ico")));
}

bool QmlDialog::event(QEvent *evnt)
{
    if(evnt->type() == QEvent::Close)
    {
        emit finished();
    }
    return QQuickWindow::event(evnt);
}
