#include "OnboardingQmlDialog.h"
#include "MegaApplication.h"
#include "mega/types.h"

#include <QEvent>
#include <QScreen>

OnboardingQmlDialog::OnboardingQmlDialog(QWindow *parent)
    : QmlDialog(parent)
    , mLoggingIn(false)
    , mCloseClicked(false)
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
    setLoggingIn(false);
    if(!close())
    {
        hide();
    }
}

bool OnboardingQmlDialog::event(QEvent *evnt)
{
    if(evnt->type() == QEvent::Close)
    {
        if(mLoggingIn)
        {
            emit closingButLoggingIn();
            evnt->ignore();
            return true;
        }
        else if(mega::NOTLOGGEDIN == MegaSyncApp->getMegaApi()->isLoggedIn()
               || mega::EPHEMERALACCOUNT == MegaSyncApp->getMegaApi()->isLoggedIn()
               || mega::CONFIRMEDACCOUNT == MegaSyncApp->getMegaApi()->isLoggedIn())
        {
            hide();
            evnt->ignore();
            return true;
        }
    }
    return QmlDialog::event(evnt);
}
