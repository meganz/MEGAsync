#include "AddBackupDialog.h"
#include "ui_AddBackupDialog.h"
#include "UserAttributesRequests/DeviceName.h"
#include "UserAttributesRequests/MyBackupsHandle.h"
#include "Utilities.h"
#include "Platform.h"
#include "DialogOpener.h"

#include "QMegaMessageBox.h"

AddBackupDialog::AddBackupDialog(QWidget *parent) :
    QDialog(parent),
    mUi(new Ui::AddBackupDialog),
    mSelectedFolder(),
    mMyBackupsFolder(),
    mDeviceNameRequest (UserAttributes::DeviceName::requestDeviceName())
{
    mUi->setupUi(this);

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
    auto processPath = [this](QStringList folderPaths)
    {
        if (!folderPaths.isEmpty())
        {
            auto folderPath = folderPaths.first();

            QString candidateDir (QDir::toNativeSeparators(QDir(folderPath).canonicalPath()));
            QString warningMessage;
            auto syncability (SyncController::isLocalFolderSyncable(candidateDir, mega::MegaSync::TYPE_BACKUP, warningMessage));

            auto finishFunc = [this, folderPath, candidateDir](QPointer<QMessageBox> msg){
                if(!msg || msg->result() == QMessageBox::Yes)
                {
                    mSelectedFolder = candidateDir;
                    mUi->folderLineEdit->setText(folderPath);
                    mUi->addButton->setEnabled(true);
                }
            };

            if (syncability == SyncController::CAN_SYNC)
            {
                finishFunc(nullptr);
            }
            else
            {
                QMegaMessageBox::MessageBoxInfo msgInfo;
                msgInfo.title = MegaSyncApp->getMEGAString();
                msgInfo.text = warningMessage;

                if (syncability == SyncController::WARN_SYNC)
                {
                    msgInfo.text += QLatin1Char('/') + tr("Do you want to continue?");
                    msgInfo.buttons = QMessageBox::Yes | QMessageBox::No;
                    msgInfo.defaultButton = QMessageBox::No;
                    msgInfo.finishFunc = finishFunc;
                    QMegaMessageBox::question(msgInfo);
                }
                else
                {
                    QMegaMessageBox::warning(msgInfo);
                }
            }
        }
    };

    SelectorInfo info;
    info.title = tr("Choose folder");
    info.parent = this;
    info.func = processPath;

    info.defaultDir = mUi->folderLineEdit->text().trimmed();
    if (!info.defaultDir.size())
    {
        info.defaultDir = Utilities::getDefaultBasePath();
    }

    info.defaultDir = QDir::toNativeSeparators(info.defaultDir);
    Platform::getInstance()->folderSelector(info);
}

void AddBackupDialog::onDeviceNameSet(const QString &devName)
{
    QString textToElide = UserAttributes::MyBackupsHandle::getMyBackupsLocalizedPath()
                          + QLatin1Char('/')
                          + devName;
    QString text = mUi->backupToLabel->fontMetrics().elidedText(textToElide, Qt::ElideMiddle, mUi->folderLineEdit->width() + mUi->changeButton->width());
    mUi->backupToLabel->setText(text);
    if(textToElide != text)
    {
        mUi->backupToLabel->setToolTip(textToElide);
    }
}

void AddBackupDialog::checkNameConflict()
{
    QStringList pathList;
    pathList.append(mSelectedFolder);

    if(!BackupNameConflictDialog::backupNamesValid(pathList))
    {
        BackupNameConflictDialog* conflictDialog = new BackupNameConflictDialog(pathList, this);
        DialogOpener::showDialog<BackupNameConflictDialog>(conflictDialog, this, &AddBackupDialog::onConflictSolved);
    }
    else
    {
        accept();
    }
}

void AddBackupDialog::onConflictSolved(QPointer<BackupNameConflictDialog> dialog)
{
    if(dialog->result() == QDialog::Accepted)
    {
        QMap<QString, QString> changes = dialog->getChanges();
        for(auto it = changes.cbegin(); it!=changes.cend(); ++it)
        {
            mBackupName = it.value();
        }
        accept();
    }
}
