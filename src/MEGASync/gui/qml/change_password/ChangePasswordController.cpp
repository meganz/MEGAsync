#include "ChangePasswordController.h"

#include "MegaApplication.h"
#include "RequestListenerManager.h"

using namespace mega;

ChangePasswordController::ChangePasswordController(QObject* parent):
    QObject(parent),
    mMegaApi(MegaSyncApp->getMegaApi())
{}

void ChangePasswordController::onRequestFinish(mega::MegaRequest* req, mega::MegaError* e)
{
    switch (req->getType())
    {
        case MegaRequest::TYPE_MULTI_FACTOR_AUTH_CHECK:
        {
            if (e->getErrorCode() == MegaError::API_OK)
            {
                if (req->getFlag()) // 2FA enabled
                {
                    emit show2FA();
                }
                else
                {
                    auto listener =
                        RequestListenerManager::instance().registerAndGetFinishListener(this, true);
                    mMegaApi->changePassword(nullptr,
                                             mPassword.toUtf8().constData(),
                                             listener.get());
                }
            }
            else
            {
                emit passwordChangeFailed(
                    QCoreApplication::translate("MegaError", e->getErrorString()));
            }
            break;
        }
        case MegaRequest::TYPE_CHANGE_PW:
        {
            if (e->getErrorCode() == MegaError::API_OK)
            {
                if (mSignOutOtherDevices)
                {
                    auto listener =
                        RequestListenerManager::instance().registerAndGetFinishListener(this, true);
                    mMegaApi->killSession(mega::INVALID_HANDLE, listener.get());
                }
                else
                {
                    emitPasswordChangeSucceed(false);
                }
            }
            else if (e->getErrorCode() == MegaError::API_EFAILED ||
                     e->getErrorCode() == MegaError::API_EEXPIRED)
            {
                emit twoFAVerificationFailed();
            }
            else if (e->getErrorCode() == MegaError::API_ETOOMANY)
            {
                emit passwordChangeFailed(tr("Too many requests. Please wait."));
            }
            else
            {
                emit passwordChangeFailed(
                    QCoreApplication::translate("MegaError", e->getErrorString()));
            }
            break;
        }
        case MegaRequest::TYPE_KILL_SESSION:
        {
            // The password is already changed at this point; the banner only
            // reflects whether the other sessions were actually signed out.
            emitPasswordChangeSucceed(e->getErrorCode() == MegaError::API_OK);
            break;
        }
    }
}

void ChangePasswordController::emitPasswordChangeSucceed(bool signedOutOtherDevices)
{
    const QString banner =
        signedOutOtherDevices ?
            tr("This includes browsers, phones and other computers — signed out even if "
               "they were offline. This app stays signed in and your syncs keep running.") :
            tr("Nothing else was signed out — any other session keeps access with your "
               "new password. To end them, sign in to mega.nz in a browser.");

    emit passwordChangeSucceed(tr("Password changed"),
                               tr("Your password has been changed"),
                               banner);
}

void ChangePasswordController::check2FA(QString pin)
{
    auto listener = RequestListenerManager::instance().registerAndGetFinishListener(this, true);
    mMegaApi->multiFactorAuthChangePassword(nullptr,
                                            mPassword.toUtf8().constData(),
                                            pin.toUtf8().constData(),
                                            listener.get());
}

void ChangePasswordController::changePassword(QString password, bool signOutOtherDevices)
{
    mSignOutOtherDevices = signOutOtherDevices;

    if (mMegaApi->checkPassword(password.toUtf8()))
    {
        emit passwordCheckFailed(tr("You have entered your current password,"
                                    " please enter a new password."));
    }
    else
    {
        mPassword = password;

        auto listener = RequestListenerManager::instance().registerAndGetFinishListener(this, true);
        char* email = mMegaApi->getMyEmail();
        if (email)
        {
            mMegaApi->multiFactorAuthCheck(email, listener.get());
            delete[] email;
        }
        else
        {
            mMegaApi->multiFactorAuthCheck(Preferences::instance()->email().toUtf8().constData(),
                                           listener.get());
        }
    }
}
