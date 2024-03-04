#include "ChangePassword.h"
#include "ui_ChangePassword.h"
#include "MegaApplication.h"
#include "QMegaMessageBox.h"
#include "DialogOpener.h"
#include "gui/Login2FA.h"

using namespace mega;

ChangePassword::ChangePassword(QWidget* parent) :
    QDialog(parent),
    mUi(new Ui::ChangePassword),
    mMegaApi (((MegaApplication*)qApp)->getMegaApi()),
    mDelegateListener (new QTMegaRequestListener(mMegaApi, this))
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

void ChangePassword::onRequestFinish(mega::MegaApi* api, mega::MegaRequest* req, mega::MegaError* e)
{
    Q_UNUSED (api)
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
                    mMegaApi->changePassword(nullptr,
                                             newPassword().toUtf8().constData(),
                                             mDelegateListener);
                }
            }
            else
            {
                QMegaMessageBox::MessageBoxInfo info;
                info.title = QMegaMessageBox::errorTitle();
                info.text = QCoreApplication::translate("MegaError", e->getErrorString());
                info.parent = this;
                QMegaMessageBox::critical(info);

                setEnabled(true);
            }
            break;
        }
        case MegaRequest::TYPE_CHANGE_PW:
        {
            if (e->getErrorCode() == MegaError::API_OK)
            {
                hide();

                QMegaMessageBox::MessageBoxInfo msgInfo;
                msgInfo.parent = parentWidget();
                msgInfo.title =  tr("Password changed");
                msgInfo.text =   tr("Your password has been changed.");
                msgInfo.finishFunc = [this](QPointer<QMessageBox>){
                    accept();
                };
                QMegaMessageBox::information(msgInfo);
            }
            else if (e->getErrorCode() == MegaError::API_EFAILED
                     || e->getErrorCode() == MegaError::API_EEXPIRED)
            {
                show2FA(true);
            }
            else if (e->getErrorCode() == MegaError::API_ETOOMANY)
            {
                setEnabled(true);

                QMegaMessageBox::MessageBoxInfo info;
                info.title = QMegaMessageBox::errorTitle();
                info.text = tr("Too many requests. Please wait.");
                info.parent = this;
                QMegaMessageBox::critical(info);
            }
            else
            {
                setEnabled(true);

                QMegaMessageBox::MessageBoxInfo info;
                info.title = QMegaMessageBox::errorTitle();
                info.text = QCoreApplication::translate("MegaError",e->getErrorString());
                info.parent = this;

                QMegaMessageBox::critical(info);
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

            mMegaApi->multiFactorAuthChangePassword(nullptr,
                                                    newPassword().toUtf8().constData(),
                                                    pin.toUtf8().constData(),
                                                    mDelegateListener);
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
    delete mDelegateListener;
}

void ChangePassword::on_bOk_clicked()
{
    const bool fieldIsEmpty{newPassword().isEmpty() || confirmNewPassword().isEmpty()};
    const bool passwordsAreEqual{!newPassword().compare(confirmNewPassword())};
    const bool newAndOldPasswordsAreTheSame{mMegaApi->checkPassword(newPassword().toUtf8())};
    const bool passwordIsWeak{mMegaApi->getPasswordStrength(newPassword().toUtf8().constData())
                == MegaApi::PASSWORD_STRENGTH_VERYWEAK};

    QMegaMessageBox::MessageBoxInfo info;
    info.title = QMegaMessageBox::errorTitle();
    info.parent = this;

    if (fieldIsEmpty)
    {
        info.text = tr("Please enter your password");
        QMegaMessageBox::warning(info);
    }
    else if (!passwordsAreEqual)
    {
        info.text = tr("The entered passwords don't match");
        QMegaMessageBox::warning(info);
    }
    else if (newAndOldPasswordsAreTheSame)
    {
        info.text = tr("You have entered your current password,"
                       " please enter a new password.");
        QMegaMessageBox::warning(info);
    }
    else if (passwordIsWeak)
    {
        info.text = tr("Please, enter a stronger password");
        QMegaMessageBox::warning(info);
    }
    else
    {
        setDisabled(true);

        char* email = mMegaApi->getMyEmail();
        if (email)
        {
            mMegaApi->multiFactorAuthCheck(email, mDelegateListener);
            delete [] email;
        }
        else
        {
            mMegaApi->multiFactorAuthCheck(Preferences::instance()->email().toUtf8().constData(),
                                           mDelegateListener);
        }
    }
}

void ChangePassword::changeEvent(QEvent *event)
{
    if (event->type() == QEvent::LanguageChange)
    {
        mUi->retranslateUi(this);
    }
    QDialog::changeEvent(event);
}
