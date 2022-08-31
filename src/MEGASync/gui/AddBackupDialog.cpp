#include "AddBackupDialog.h"
#include "ui_AddBackupDialog.h"
#include "QMegaMessageBox.h"
#include "UserAttributesRequests/DeviceName.h"

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
    mUi->backupToLabel->setText(mMyBackupsFolder + mDeviceNameRequest->getDeviceName());
}

QString AddBackupDialog::getSelectedFolder()
{
    return mSelectedFolder;
}

void AddBackupDialog::on_changeButton_clicked()
{
    QString folderPath = QFileDialog::getExistingDirectory(this, tr("Choose folder"),
                                                           QDir::home().path(),
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
