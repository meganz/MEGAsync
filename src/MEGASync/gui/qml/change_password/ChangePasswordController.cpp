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
                // The API invalidates every other session as part of the
                // password change itself (same server path as `usr`), so no
                // explicit killSession is needed and the outcome is always the
                // same: only this session survives. SNC-6910
                emit passwordChangeSucceed(
                    tr("Password changed"),
                    tr("Your password has been changed"),
                    tr("This includes browsers, phones and other computers — signed out even "
                       "if they were offline. This app stays signed in and your syncs keep "
                       "running."));
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
    }
}

void ChangePasswordController::check2FA(QString pin)
{
    auto listener = RequestListenerManager::instance().registerAndGetFinishListener(this, true);
    mMegaApi->multiFactorAuthChangePassword(nullptr,
                                            mPassword.toUtf8().constData(),
                                            pin.toUtf8().constData(),
                                            listener.get());
}

void ChangePasswordController::changePassword(QString password)
{
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
