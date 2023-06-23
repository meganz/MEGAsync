#include "QmlDialog.h"
#include <QEvent>
#include <QScreen>
#include <QApplication>

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
        if(!value && mCloseClicked)
        {
            close();
        }
        emit loggingInChanged();
    }
}

bool QmlDialog::event(QEvent *evnt)
{
    if(evnt->type() == QEvent::Close)
    {
        if(mLoggingIn)
        {
            mCloseClicked = true;
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
