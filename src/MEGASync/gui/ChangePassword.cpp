#include "ChangePassword.h"

#include "DialogOpener.h"
#include "Login2FA.h"
#include "MegaApplication.h"
#include "MessageDialogOpener.h"
#include "RefreshAppChangeEvent.h"
#include "RequestListenerManager.h"
#include "ui_ChangePassword.h"

using namespace mega;

ChangePassword::ChangePassword(QWidget* parent)
    : QDialog(parent)
    , mUi(new Ui::ChangePassword)
    , mMegaApi (((MegaApplication*)qApp)->getMegaApi())
{
    mUi->setupUi(this);
    mUi->bOk->setDefault(true);
}

QString ChangePassword::newPassword()
{
    return mUi->lNewPassword->text();
}

QString ChangePassword::confirmNewPassword()
{
    return mUi->lConfirmNewPassword->text();
}

void ChangePassword::onRequestFinish(mega::MegaRequest* req, mega::MegaError* e)
{
    switch(req->getType())
    {
        case MegaRequest::TYPE_MULTI_FACTOR_AUTH_CHECK:
        {
            if (e->getErrorCode() == MegaError::API_OK)
            {
                if (req->getFlag()) //2FA enabled
                {
                    show2FA(false);
                }
                else
                {
                    auto listener = RequestListenerManager::instance().registerAndGetFinishListener(this, true);
                    mMegaApi->changePassword(nullptr,
                                             newPassword().toUtf8().constData(),
                                             listener.get());
                }
            }
            else
            {
                MessageDialogInfo info;
                info.descriptionText =
                    QCoreApplication::translate("MegaError", e->getErrorString());
                info.parent = this;
                MessageDialogOpener::critical(info);

                setEnabled(true);
            }
            break;
        }
        case MegaRequest::TYPE_CHANGE_PW:
        {
            if (e->getErrorCode() == MegaError::API_OK)
            {
                hide();

                MessageDialogInfo msgInfo;
                msgInfo.parent = parentWidget();
                msgInfo.titleText = tr("Password changed");
                msgInfo.descriptionText = tr("Your password has been changed.");
                msgInfo.finishFunc = [this](QPointer<MessageDialogResult>)
                {
                    accept();
                };
                MessageDialogOpener::information(msgInfo);
            }
            else if (e->getErrorCode() == MegaError::API_EFAILED ||
                     e->getErrorCode() == MegaError::API_EEXPIRED)
            {
                show2FA(true);
            }
            else if (e->getErrorCode() == MegaError::API_ETOOMANY)
            {
                setEnabled(true);

                MessageDialogInfo info;
                info.descriptionText = tr("Too many requests. Please wait.");
                info.parent = this;
                MessageDialogOpener::critical(info);
            }
            else
            {
                setEnabled(true);

                MessageDialogInfo info;
                info.descriptionText =
                    QCoreApplication::translate("MegaError", e->getErrorString());
                info.parent = this;

                MessageDialogOpener::critical(info);
            }
            break;
        }
    }
}

void ChangePassword::show2FA(bool invalidCode)
{
    QPointer<Login2FA> verification = new Login2FA(this);
    verification->invalidCode(invalidCode);
    DialogOpener::showDialog<Login2FA>(verification, [verification, this]()
    {
        if (verification->result() == QDialog::Accepted)//need to check if verificaiton is valid??
        {
            QString pin = verification->pinCode();
            setDisabled(true);

            auto listener = RequestListenerManager::instance().registerAndGetFinishListener(this, true);
            mMegaApi->multiFactorAuthChangePassword(nullptr,
                                                    newPassword().toUtf8().constData(),
                                                    pin.toUtf8().constData(),
                                                    listener.get());
        }
        else
        {
            setEnabled(true);
        }
    });
}

ChangePassword::~ChangePassword()
{
    delete mUi;
}

void ChangePassword::on_bOk_clicked()
{
    const bool fieldIsEmpty{newPassword().isEmpty() || confirmNewPassword().isEmpty()};
    const bool passwordsAreEqual{!newPassword().compare(confirmNewPassword())};
    const bool newAndOldPasswordsAreTheSame{mMegaApi->checkPassword(newPassword().toUtf8())};
    const bool passwordIsWeak{mMegaApi->getPasswordStrength(newPassword().toUtf8().constData())
                == MegaApi::PASSWORD_STRENGTH_VERYWEAK};

    MessageDialogInfo info;
    info.parent = this;

    if (fieldIsEmpty)
    {
        info.descriptionText = tr("Please enter your password");
        MessageDialogOpener::warning(info);
    }
    else if (!passwordsAreEqual)
    {
        info.descriptionText = tr("The entered passwords don't match");
        MessageDialogOpener::warning(info);
    }
    else if (newAndOldPasswordsAreTheSame)
    {
        info.descriptionText = tr("You have entered your current password,"
                                  " please enter a new password.");
        MessageDialogOpener::warning(info);
    }
    else if (passwordIsWeak)
    {
        info.descriptionText = tr("Please, enter a stronger password");
        MessageDialogOpener::warning(info);
    }
    else
    {
        setDisabled(true);

        auto listener = RequestListenerManager::instance().registerAndGetFinishListener(this, true);
        char* email = mMegaApi->getMyEmail();
        if (email)
        {
            mMegaApi->multiFactorAuthCheck(email, listener.get());
            delete [] email;
        }
        else
        {
            mMegaApi->multiFactorAuthCheck(Preferences::instance()->email().toUtf8().constData(),
                                           listener.get());
        }
    }
}

bool ChangePassword::event(QEvent* event)
{
    if (RefreshAppChangeEvent::isRefreshEvent(event))
    {
        mUi->retranslateUi(this);
    }
    return QDialog::event(event);
}
