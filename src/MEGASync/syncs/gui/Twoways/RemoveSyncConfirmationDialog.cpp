#include "RemoveSyncConfirmationDialog.h"

#include "ui_RemoveSyncConfirmationDialog.h"

RemoveSyncConfirmationDialog::RemoveSyncConfirmationDialog(QWidget *parent)
    : QDialog(parent)
    , ui(new Ui::RemoveSyncConfirmationDialog)
{
    ui->setupUi(this);

    const QString label = QString::fromLatin1(" ") + tr("Remove");
    ui->bRemove->setText(label);
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
