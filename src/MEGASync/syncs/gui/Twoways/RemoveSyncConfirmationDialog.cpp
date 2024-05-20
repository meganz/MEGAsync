#include "RemoveSyncConfirmationDialog.h"
#include "ui_RemoveSyncConfirmationDialog.h"

RemoveSyncConfirmationDialog::RemoveSyncConfirmationDialog(QWidget *parent)
    : QDialog(parent)
    , ui(new Ui::RemoveSyncConfirmationDialog)
{
    ui->setupUi(this);

    // Translations
    setWindowTitle(tr("Remove sync?"));
    ui->labelHeader->setText(tr("Remove this sync?"));
    ui->labelMessage->setText(tr("The data on your computer and in MEGA will not be removed,\n but the folders will no longer sync with each other."));
    ui->bOK->setText(tr("Remove"));
    ui->bCancel->setText(tr("Cancel"));

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

