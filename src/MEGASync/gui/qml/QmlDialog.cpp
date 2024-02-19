#include "QmlDialog.h"

#include <QEvent>

QmlDialog::QmlDialog(QWindow *parent)
    : QQuickWindow(parent)
    , mIconSrc(QString::fromUtf8(":/images/app_ico.ico")) // Default mega app icon
{
    setFlags(flags() | Qt::Dialog);
    connect(this, &QmlDialog::requestPageFocus, this, &QmlDialog::onRequestPageFocus, Qt::QueuedConnection);
}

void QmlDialog::setIconSrc(const QString& iconSrc)
{
    QString source = iconSrc;
    if(iconSrc.startsWith(QString::fromUtf8("qrc:")))
    {
        source = source.mid(3);
    }

    if(source != mIconSrc)
    {
        mIconSrc = source;
        setIcon(QIcon(mIconSrc));
    }
}

bool QmlDialog::event(QEvent *evnt)
{
    if(evnt->type() == QEvent::Close)
    {
        emit finished();
    }

    return QQuickWindow::event(evnt);
}

void QmlDialog::onRequestPageFocus()
{
    emit initializePageFocus();
}
