#include "AddBackupDialog.h"
#include "ui_AddBackupDialog.h"
#include "QMegaMessageBox.h"
#include "UserAttributesRequests/DeviceName.h"
#include "Utilities.h"

#include <QFileDialog>

AddBackupDialog::AddBackupDialog(QWidget *parent) :
    QDialog(parent),
    mUi(new Ui::AddBackupDialog),
    mSelectedFolder(),
    mMyBackupsFolder(),
    mDeviceNameRequest (UserAttributes::DeviceName::requestDeviceName())
{
    mUi->setupUi(this);
    setWindowFlags(windowFlags() & ~Qt::WindowContextHelpButtonHint);

    connect(mDeviceNameRequest.get(), &UserAttributes::DeviceName::attributeReady,
            this, &AddBackupDialog::onDeviceNameSet);
    onDeviceNameSet(mDeviceNameRequest->getDeviceName());

#ifdef Q_OS_MACOS
    // Display our modal dialog embedded title label when parent is set
    mUi->embeddedTitleLabel->setVisible(this->parent() != nullptr);
#endif

    connect(mUi->addButton, &QPushButton::clicked, this, &AddBackupDialog::checkNameConflict);
    connect(mUi->cancelButton, &QPushButton::clicked, this, &QDialog::reject);
}

AddBackupDialog::~AddBackupDialog()
{
    delete mUi;
}

void AddBackupDialog::setMyBackupsFolder(const QString& folder)
{
    mMyBackupsFolder = folder;
    mUi->backupToLabel->setText(mMyBackupsFolder + mDeviceNameRequest->getDeviceName());
}

void AddBackupDialog::setMyBackupsFolderHandle(const mega::MegaHandle& handle)
{
    mMyBackupsHandle = handle;
}

QString AddBackupDialog::getSelectedFolder()
{
    return mSelectedFolder;
}

QString AddBackupDialog::getBackupName()
{
    return mBackupName;
}

void AddBackupDialog::on_changeButton_clicked()
{
    QString folderPath = QFileDialog::getExistingDirectory(this, tr("Choose folder"),
                                                           Utilities::getDefaultBasePath(),
                                                           QFileDialog::DontResolveSymlinks);
    if (!folderPath.isEmpty())
    {
        QString candidateDir (QDir::toNativeSeparators(QDir(folderPath).canonicalPath()));
        QString warningMessage;
        auto syncability (SyncController::isLocalFolderSyncable(candidateDir, mega::MegaSync::TYPE_BACKUP, warningMessage));

        if (syncability == SyncController::CANT_SYNC)
        {
            QMegaMessageBox::warning(nullptr, QString(), warningMessage, QMessageBox::Ok);
        }
        else if (syncability == SyncController::CAN_SYNC
                 || (syncability == SyncController::WARN_SYNC
                     && QMegaMessageBox::warning(nullptr, QString(), warningMessage
                                                 + QLatin1Char('/')
                                                 + tr("Do you want to continue?"),
                                                 QMessageBox::Yes | QMessageBox::No, QMessageBox::No)
                     == QMessageBox::Yes))
        {
            mSelectedFolder = candidateDir;
            mUi->folderLineEdit->setText(folderPath);
            mUi->addButton->setEnabled(true);
        }
    }
}

void AddBackupDialog::onDeviceNameSet(const QString &devName)
{
    mUi->backupToLabel->setText(mMyBackupsFolder + devName);
}

void AddBackupDialog::checkNameConflict()
{
    QStringList pathList;
    pathList.append(mSelectedFolder);
    if(!BackupNameConflictDialog::backupNamesValid(pathList, mMyBackupsHandle))
    {
        BackupNameConflictDialog* conflictDialog = new BackupNameConflictDialog(pathList, mMyBackupsHandle, this);
        connect(conflictDialog, &BackupNameConflictDialog::accepted,
                this, &AddBackupDialog::onConflictSolved);
    }
    else
    {
        accept();
    }
}

void AddBackupDialog::onConflictSolved()
{
    auto conflictDialog = qobject_cast<BackupNameConflictDialog*>(sender());
    QMap<QString, QString> changes = conflictDialog->getChanges();
    for(auto it = changes.cbegin(); it!=changes.cend(); ++it)
    {
        mBackupName = it.value();
    }
    accept();
}
