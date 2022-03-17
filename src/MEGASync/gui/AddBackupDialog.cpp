#include "AddBackupDialog.h"
#include "ui_AddBackupDialog.h"

#include <QFileDialog>

AddBackupDialog::AddBackupDialog(QWidget *parent) :
    QDialog(parent),
    mUi(new Ui::AddBackupDialog),
    mSelectedFolder(QDir::home()),
    mMyBackupsFolder(),
    mSyncController(),
    mDeviceName()
{
    mUi->setupUi(this);

#ifdef Q_OS_MACOS
    // Display our modal dialog embedded title label when parent is set
    mUi->embeddedTitleLabel->setVisible(this->parent() != nullptr);
#endif

    connect(&mSyncController, &SyncController::deviceName,
            this, &AddBackupDialog::onDeviceNameSet);

    mSyncController.getDeviceName();

    connect(mUi->addButton, &QPushButton::clicked, this, &QDialog::accept);
    connect(mUi->cancelButton, &QPushButton::clicked, this, &QDialog::reject);
}

AddBackupDialog::~AddBackupDialog()
{
    delete mUi;
}

void AddBackupDialog::setMyBackupsFolder(const QString& folder)
{
    mMyBackupsFolder = folder;
    mUi->backupToLabel->setText(mMyBackupsFolder + QLatin1Char('/') + mDeviceName);
}

QDir AddBackupDialog::getSelectedFolder()
{
    return mSelectedFolder;
}

void AddBackupDialog::on_changeButton_clicked()
{
    QString folderPath = QFileDialog::getExistingDirectory(this, tr("Choose Folder"),
                                                    QDir::home().path(),
                                                    QFileDialog::DontResolveSymlinks);
    if(folderPath.isEmpty())
        return;

    mSelectedFolder = folderPath;
    mUi->folderLineEdit->setText(mSelectedFolder.path());
    mUi->addButton->setEnabled(true);
}

void AddBackupDialog::onDeviceNameSet(const QString& devName)
{
    mDeviceName = devName;
    mUi->backupToLabel->setText(mMyBackupsFolder + QLatin1Char('/') + mDeviceName);
}

