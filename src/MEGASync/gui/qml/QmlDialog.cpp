#include "QmlDialog.h"

#include <QEvent>

QmlDialog::QmlDialog(QWindow *parent)
    : QQuickWindow(parent)
    , mIconScr(QString::fromUtf8(":/images/app_ico.ico")) // Default mega app icon
{
    setFlags(flags() | Qt::Dialog);
    connect(this, &QmlDialog::requestPageFocus, this, &QmlDialog::onRequestPageFocus, Qt::QueuedConnection);
}

void QmlDialog::setIconScr(const QString& iconScr)
{
    QString source = iconScr;
    if(iconScr.startsWith(QString::fromUtf8("qrc:")))
    {
        source = source.mid(3);
    }

    if(source != mIconScr)
    {
        mIconScr = source;
        setIcon(QIcon(mIconScr));
    }
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
