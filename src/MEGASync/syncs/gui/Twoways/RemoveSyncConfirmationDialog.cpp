#include "RemoveSyncConfirmationDialog.h"
#include "ui_RemoveSyncConfirmationDialog.h"

RemoveSyncConfirmationDialog::RemoveSyncConfirmationDialog(QWidget *parent)
    : QDialog(parent)
    , ui(new Ui::RemoveSyncConfirmationDialog)
{
    ui->setupUi(this);

    ui->bOK->setDefault(true);
    setFocusProxy(ui->bOK);
}

RemoveSyncConfirmationDialog::~RemoveSyncConfirmationDialog()
{
    delete ui;
}

void RemoveSyncConfirmationDialog::on_bOK_clicked()
{
    accept();
}
