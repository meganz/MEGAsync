#include "ConfirmSSLexception.h"
#include "ui_ConfirmSSLexception.h"

ConfirmSSLexception::ConfirmSSLexception(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::ConfirmSSLexception)
{
    ui->setupUi(this);
    setAttribute(Qt::WA_QuitOnClose, false);
}

ConfirmSSLexception::~ConfirmSSLexception()
{
    delete ui;
}

bool ConfirmSSLexception::isDefaultDownloadOption()
{
    return ui->cRemember->isChecked();
}

void ConfirmSSLexception::changeEvent(QEvent *event)
{
    if (event->type() == QEvent::LanguageChange)
    {
        ui->retranslateUi(this);
    }
    QDialog::changeEvent(event);
}
