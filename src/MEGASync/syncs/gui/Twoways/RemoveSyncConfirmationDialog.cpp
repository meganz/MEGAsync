#include "RemoveSyncConfirmationDialog.h"
#include "ui_RemoveSyncConfirmationDialog.h"

RemoveSyncConfirmationDialog::RemoveSyncConfirmationDialog(QWidget *parent)
    : QDialog(parent)
    , ui(new Ui::RemoveSyncConfirmationDialog)
{
    ui->setupUi(this);

    ui->bRemove->setDefault(true);
    setFocusProxy(ui->bRemove);
}

RemoveSyncConfirmationDialog::~RemoveSyncConfirmationDialog()
{
    delete ui;
}

void RemoveSyncConfirmationDialog::on_bRemove_clicked()
{
    accept();
}
