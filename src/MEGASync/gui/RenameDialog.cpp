#include "RenameDialog.h"
#include "ui_RenameDialog.h"

#include <QPushButton>

RenameDialog::RenameDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::RenameDialog)
{
    ui->setupUi(this);

    ui->buttonBox->button(QDialogButtonBox::Ok)->setText(tr("Rename"));
}

RenameDialog::~RenameDialog()
{
    delete ui;
}

void RenameDialog::setMessage(const QString &message)
{
    ui->message->setText(message);
}

void RenameDialog::setEditorValidator(const QValidator *v)
{
    ui->leEditor->setValidator(v);
}

QString RenameDialog::getRenameText()
{
    return ui->leEditor->text();
}
