#include "QmlDialog.h"
#include "MegaApplication.h"

#include <QEvent>
#include <QScreen>

QmlDialog::QmlDialog(QWindow *parent)
    : QQuickWindow(parent)
    , mLoggingIn(false)
    , mCloseClicked(false)
{
}

QmlDialog::~QmlDialog()
{
}

bool QmlDialog::getLoggingIn() const
{
    return mLoggingIn;
}

void QmlDialog::setLoggingIn(bool value)
{
    if(mLoggingIn != value)
    {
        mLoggingIn = value;
        emit loggingInChanged();
    }
}

void QmlDialog::forceClose()
{
    setLoggingIn(false);
    qDebug()<<parent();
    close();
}

bool QmlDialog::event(QEvent *evnt)
{
    if(evnt->type() == QEvent::Close)
    {
        if(mLoggingIn)
        {
            emit closingButLoggingIn();
            return true;
        }
        else if(!MegaSyncApp->getMegaApi()->isLoggedIn())
        {
            hide();
            return true;
        }
        else
        {
            emit finished();
        }
    }
    return QQuickWindow::event(evnt);
}
