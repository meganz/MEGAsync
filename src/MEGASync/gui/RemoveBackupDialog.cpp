#include "RemoveBackupDialog.h"
#include "ui_RemoveBackupDialog.h"

#ifdef Q_OS_MACOS
#include <QOperatingSystemVersion>
#endif

RemoveBackupDialog::RemoveBackupDialog(std::shared_ptr<SyncSetting> backup, QWidget *parent) :
    QDialog(parent),
    mUi(new Ui::RemoveBackupDialog),
    mBackup(backup)
{
    mUi->setupUi(this);

#ifdef Q_OS_MACOS
    // Display our modal dialog embedded title label on macOS Big Sur and onwards
    bool isMacOSBigSur = QOperatingSystemVersion::current() > QOperatingSystemVersion::MacOSCatalina;
    mUi->macOSBigSurTitleLabel->setVisible(isMacOSBigSur);
#endif

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
