#include "QmlDialog.h"

#include <QEvent>

QmlDialog::QmlDialog(QWindow *parent)
    : QQuickWindow(parent)
{
    setFlags(flags() | Qt::Dialog);
    setIcon(QIcon(QString::fromUtf8("://images/app_ico.ico")));

    connect(this, &QmlDialog::requestPageFocus, this, &QmlDialog::onRequestPageFocus, Qt::QueuedConnection);
}

bool QmlDialog::event(QEvent *evnt)
{
    if(evnt->type() == QEvent::Close)
    {
        emit finished();
    }
    else if (evnt->type() == QEvent::LanguageChange)
    {
        emit languageChanged();
    }

    return QQuickWindow::event(evnt);
}

void QmlDialog::onRequestPageFocus()
{
    emit initializePageFocus();
}
