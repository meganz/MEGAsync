#include "RemoveBackupDialog.h"
#include "ui_RemoveBackupDialog.h"

RemoveBackupDialog::RemoveBackupDialog(std::shared_ptr<SyncSetting> backup, QWidget *parent) :
    QDialog(parent),
    mUi(new Ui::RemoveBackupDialog),
    mBackup(backup)
{
    mUi->setupUi(this);

    connect(mUi->confirmButton, &QPushButton::clicked, this, &QDialog::accept);
    connect(mUi->cancelButton, &QPushButton::clicked, this, &QDialog::reject);

    mUi->textLabel->setText(mUi->textLabel->text().arg(backup->name()));
}

RemoveBackupDialog::~RemoveBackupDialog()
{
    delete mUi;
}

std::shared_ptr<SyncSetting> RemoveBackupDialog::backupToRemove()
{
    return mBackup;
}

bool RemoveBackupDialog::alsoRemoveMEGAFolder()
{
    return mUi->removeRemoteCheckBox->isChecked();
}
