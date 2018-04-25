#include "ChangePassword.h"
#include <QMessageBox>
#include "ui_ChangePassword.h"
#include "MegaApplication.h"

using namespace mega;

ChangePassword::ChangePassword(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::ChangePassword)
{
    ui->setupUi(this);
    setAttribute(Qt::WA_QuitOnClose, false);
    setWindowFlags(windowFlags() & ~Qt::WindowContextHelpButtonHint);

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
        case MegaRequest::TYPE_CHANGE_PW:
        if (e->getErrorCode() == MegaError::API_OK)
        {
            QMessageBox::information(this, tr("Password changed"), tr("Your password has been changed."));
            accept();
        }
        else
        {
            QMessageBox::warning(this, tr("Error"), tr("Wrong password"));
        }

        break;
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
    else if (newPassword().size() < 8)
    {
        QMessageBox::warning(this, tr("Error"), tr("Please, enter a stronger password"));
        return;
    }

    megaApi->changePassword(NULL, newPassword().toUtf8().constData(), delegateListener);
}

void ChangePassword::changeEvent(QEvent *event)
{
    if (event->type() == QEvent::LanguageChange)
    {
        ui->retranslateUi(this);
    }
    QDialog::changeEvent(event);
}
