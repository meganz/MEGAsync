#include "MessageBox.h"
#include "ui_MessageBox.h"

MessageBox::MessageBox(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::MessageBox)
{
    ui->setupUi(this);
    setAttribute(Qt::WA_QuitOnClose, false);
    setWindowFlags(windowFlags() & ~Qt::WindowContextHelpButtonHint);

    ui->cDefaultOption->setChecked(false);
    ui->bOK->setEnabled(true);
    ui->bOK->setDefault(true);
}

MessageBox::~MessageBox()
{
    delete ui;
}

bool MessageBox::dontAskAgain()
{
    return ui->cDefaultOption->isChecked();
}

void MessageBox::changeEvent(QEvent *event)
{
    if (event->type() == QEvent::LanguageChange)
    {
        ui->retranslateUi(this);
    }
    QDialog::changeEvent(event);
}

