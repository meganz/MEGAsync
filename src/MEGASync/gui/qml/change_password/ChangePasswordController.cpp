#include "ChangePasswordController.h"

#include "MegaApplication.h"
#include "MessageDialogOpener.h"
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
                emit passwordChangeFailed();

                MessageDialogInfo info;
                info.descriptionText =
                    QCoreApplication::translate("MegaError", e->getErrorString());
                MessageDialogOpener::critical(info);
            }
            break;
        }
        case MegaRequest::TYPE_CHANGE_PW:
        {
            if (e->getErrorCode() == MegaError::API_OK)
            {
                emit passwordChangeSucceed();

                MessageDialogInfo msgInfo;
                msgInfo.titleText = tr("Password changed");
                msgInfo.descriptionText = tr("Your password has been changed.");
                MessageDialogOpener::information(msgInfo);
            }
            else if (e->getErrorCode() == MegaError::API_EFAILED ||
                     e->getErrorCode() == MegaError::API_EEXPIRED)
            {
                emit twoFAVerificationFailed();
            }
            else if (e->getErrorCode() == MegaError::API_ETOOMANY)
            {
                MessageDialogInfo info;
                info.descriptionText = tr("Too many requests. Please wait.");
                MessageDialogOpener::critical(info);
            }
            else
            {
                emit passwordChangeFailed();

                MessageDialogInfo info;
                info.descriptionText =
                    QCoreApplication::translate("MegaError", e->getErrorString());

                MessageDialogOpener::critical(info);
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

void ChangePasswordController::changePassword(QString password, QString confirmPassword)
{
    MessageDialogInfo info;

    if (password.isEmpty() || confirmPassword.isEmpty())
    {
        info.descriptionText = tr("Please enter your password");
        MessageDialogOpener::warning(info);

        emit passwordChangeFailed();
    }
    else if (password.compare(confirmPassword))
    {
        info.descriptionText = tr("The entered passwords don't match");
        MessageDialogOpener::warning(info);

        emit passwordChangeFailed();
    }
    else if (mMegaApi->checkPassword(password.toUtf8())) // new and old pass are the equals.
    {
        info.descriptionText = tr("You have entered your current password,"
                                  " please enter a new password.");
        MessageDialogOpener::warning(info);

        emit passwordChangeFailed();
    }
    else if (mMegaApi->getPasswordStrength(password.toUtf8().constData()) ==
             MegaApi::PASSWORD_STRENGTH_VERYWEAK)
    {
        info.descriptionText = tr("Please, enter a stronger password");
        MessageDialogOpener::warning(info);

        emit passwordChangeFailed();
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
