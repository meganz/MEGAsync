#include "ChangePassword.h"
#include <QMessageBox>
#include "ui_ChangePassword.h"
#include "MegaApplication.h"
#include "gui/Login2FA.h"

using namespace mega;

ChangePassword::ChangePassword(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::ChangePassword)
{
    ui->setupUi(this);
    setAttribute(Qt::WA_QuitOnClose, false);
    setWindowFlags(windowFlags() & ~Qt::WindowContextHelpButtonHint);
    ui->bOk->setDefault(true);

    megaApi = ((MegaApplication *)qApp)->getMegaApi();
    delegateListener = new QTMegaRequestListener(megaApi, this);
}

QString ChangePassword::newPassword()
{
    return ui->lNewPassword->text();
}

QString ChangePassword::confirmNewPassword()
{
    return ui->lConfirmNewPassword->text();
}

void ChangePassword::onRequestFinish(mega::MegaApi *api, mega::MegaRequest *request, mega::MegaError *e)
{
    switch(request->getType())
    {
        case MegaRequest::TYPE_MULTI_FACTOR_AUTH_CHECK:
        {
            if (e->getErrorCode() == MegaError::API_OK)
            {
                if (request->getFlag()) //2FA enabled
                {
                    QPointer<ChangePassword> dialog = this;
                    QPointer<Login2FA> verification = new Login2FA(this);
                    int result = verification->exec();
                    if (!dialog || !verification || result != QDialog::Accepted)
                    {
                        if (dialog)
                        {
                            ui->bOk->setEnabled(true);
                        }
                        delete verification;
                        return;
                    }

                    QString pin = verification->pinCode();
                    delete verification;

                    megaApi->multiFactorAuthChangePassword(NULL, newPassword().toUtf8().constData(), pin.toUtf8().constData(), delegateListener);
                }
                else
                {
                    megaApi->changePassword(NULL, newPassword().toUtf8().constData(), delegateListener);
                }
            }
            else
            {
                QMessageBox::critical(this, tr("Error"), QCoreApplication::translate("MegaError", e->getErrorString()));
            }
            break;
        }
        case MegaRequest::TYPE_CHANGE_PW:
        {
            if (e->getErrorCode() == MegaError::API_OK)
            {
                ui->bOk->setEnabled(true);
                accept();
            }
            else if (e->getErrorCode() == MegaError::API_EFAILED || e->getErrorCode() == MegaError::API_EEXPIRED)
            {
                QPointer<ChangePassword> dialog = this;
                QPointer<Login2FA> verification = new Login2FA(this);
                verification->invalidCode(true);
                int result = verification->exec();
                if (!dialog || !verification || result != QDialog::Accepted)
                {
                    if (dialog)
                    {
                        ui->bOk->setEnabled(true);
                    }
                    delete verification;
                    return;
                }

                QString pin = verification->pinCode();
                delete verification;

                megaApi->multiFactorAuthChangePassword(NULL, newPassword().toUtf8().constData(), pin.toUtf8().constData(), delegateListener);
            }
            else if (e->getErrorCode() == MegaError::API_ETOOMANY)
            {
                ui->bOk->setEnabled(true);
                QMessageBox::critical(NULL, tr("Error"), tr("Too many requests. Please wait."));
            }
            else
            {
                ui->bOk->setEnabled(true);
                QMessageBox::critical(this, tr("Error"), QCoreApplication::translate("MegaError", e->getErrorString()));
            }

            break;
        }
    }
}

ChangePassword::~ChangePassword()
{
    delete ui;
    delete delegateListener;
}

void ChangePassword::on_bOk_clicked()
{
    bool emptyField = newPassword().isEmpty() || confirmNewPassword().isEmpty();
    bool equalPasswords = !newPassword().compare(confirmNewPassword());

    if (emptyField)
    {
        QMessageBox::warning(this, tr("Error"), tr("Please enter your password"));
        return;
    }
    else if (!equalPasswords)
    {
        QMessageBox::warning(this, tr("Error"), tr("The entered passwords don't match"));
        return;
    }
    else if (megaApi->getPasswordStrength(newPassword().toUtf8().constData()) == MegaApi::PASSWORD_STRENGTH_VERYWEAK)
    {
        QMessageBox::warning(this, tr("Error"), tr("Please, enter a stronger password"));
        return;
    }

    ui->bOk->setEnabled(false);

    char *email = megaApi->getMyEmail();
    if (email)
    {
        megaApi->multiFactorAuthCheck(email, delegateListener);
        delete [] email;
    }
    else
    {
        megaApi->multiFactorAuthCheck(Preferences::instance()->email().toUtf8().constData(), delegateListener);
    }
}

void ChangePassword::changeEvent(QEvent *event)
{
    if (event->type() == QEvent::LanguageChange)
    {
        ui->retranslateUi(this);
    }
    QDialog::changeEvent(event);
}
