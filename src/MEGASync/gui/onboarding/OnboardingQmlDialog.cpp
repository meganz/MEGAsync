#include "OnboardingQmlDialog.h"
#include "MegaApplication.h"
#include "mega/types.h"
#include "LoginController.h"

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

void OnboardingQmlDialog::raise()
{
    // The following four lines are required by Ubuntu to bring the window to the front and
    // move it to the center of the current screen, if the screen is a part of a virtual desktop or multiple screen
    // we will need add the current screen offset(topleft) to the calculated central position.
    const auto& geometry = QmlDialog::screen()->geometry();
    int xPos = geometry.x() + static_cast<int>(geometry.width() * 0.5 - width() * 0.5);
    int yPos = geometry.y() + static_cast<int>(geometry.height() * 0.5 - height() * 0.5);

    hide();
    QmlDialog::setPosition(xPos, yPos);
    show();

    // The following two lines are required by Windows (activate) and macOS (raise)
    QmlDialog::requestActivate();
    QmlDialog::raise();
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
        else
        {
            MegaSyncApp->getLoginController()->processOnboardingClosed();
        }
    }
    return QmlDialog::event(evnt);
}
