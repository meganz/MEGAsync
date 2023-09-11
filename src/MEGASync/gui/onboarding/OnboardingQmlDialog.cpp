#include "OnboardingQmlDialog.h"
#include "MegaApplication.h"
#include "mega/types.h"

#include <QEvent>
#include <QScreen>

OnboardingQmlDialog::OnboardingQmlDialog(QWindow *parent)
    : QmlDialog(parent)
    , mLoggingIn(false)
    , mCloseClicked(false)
    , mForceClose(false)
{
}

OnboardingQmlDialog::~OnboardingQmlDialog()
{
}

bool OnboardingQmlDialog::getLoggingIn() const
{
    return mLoggingIn;
}

void OnboardingQmlDialog::setLoggingIn(bool value)
{
    if(mLoggingIn != value)
    {
        mLoggingIn = value;
        emit loggingInChanged();
    }
}

void OnboardingQmlDialog::forceClose()
{
    mForceClose = true;
    if(!close())
    {
        hide();
    }
}

bool OnboardingQmlDialog::event(QEvent *evnt)
{
    if(evnt->type() == QEvent::WindowUnblocked && mForceClose)
    {
        close();
    }
    else if(evnt->type() == QEvent::Close && !mForceClose)
    {
        if(mLoggingIn)
        {
            emit closingButLoggingIn();
            evnt->ignore();
            return true;
        }
        else if(mega::EPHEMERALACCOUNT == MegaSyncApp->getMegaApi()->isLoggedIn())
        {
            emit closingButCreatingAccount();
            evnt->ignore();
            return true;
        }
    }
    return QmlDialog::event(evnt);
}
