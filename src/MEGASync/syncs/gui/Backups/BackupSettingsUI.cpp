#include "BackupSettingsUI.h"

#include "syncs/gui/Backups/BackupTableView.h"
#include "syncs/model/BackupItemModel.h"

#include "qml/QmlDialogWrapper.h"

#include "backups/Backups.h"

#include "Utilities.h"
#include "DialogOpener.h"
#include "RemoveBackupDialog.h"
#include "QMegaMessageBox.h"

#include "ui_SyncSettingsUIBase.h"

BackupSettingsUI::BackupSettingsUI(QWidget *parent) :
    SyncSettingsUIBase(parent)
{
    setBackupsTitle();
    setTable<BackupTableView, BackupItemModel>();

    connect(mSyncController, &SyncController::backupMoveOrRemoveRemoteFolderError, this, [this](std::shared_ptr<mega::MegaError> err)
    {
        onSavingSyncsCompleted(SAVING_FINISHED);
        QMegaMessageBox::MessageBoxInfo msgInfo;
        msgInfo.title = tr("Error moving or removing remote backup folder");
        msgInfo.text = tr("Failed to move or remove the remote backup folder. Reason: %1")
                .arg(QCoreApplication::translate("MegaError", err->getErrorString()));
        QMegaMessageBox::warning(msgInfo);

    });

    mElements.initElements(this);
    ui->gSyncs->setUsePermissions(false);
}

BackupSettingsUI::~BackupSettingsUI()
{
}

void BackupSettingsUI::addButtonClicked(mega::MegaHandle)
{
    if(auto dialog = DialogOpener::findDialog<QmlDialogWrapper<Backups>>())
    {
        auto backupsDialog = dialog->getDialog();
        DialogOpener::showDialog(backupsDialog, [this, backupsDialog]()
        {
            if(backupsDialog->result() == QDialog::Accepted)
            {
                syncsStateInformation(SyncStateInformation::SAVING);
            }
        });
    }
    else
    {
        QPointer<QmlDialogWrapper<Backups>> backupsDialog = new QmlDialogWrapper<Backups>();
        DialogOpener::showDialog(backupsDialog, [this, backupsDialog]()
        {
            if(backupsDialog->result() == QDialog::Accepted)
            {
                syncsStateInformation(SyncStateInformation::SAVING);
            }
        });
    }
}

void BackupSettingsUI::changeEvent(QEvent *event)
{    
    if(event->type() == QEvent::LanguageChange)
    {
        mElements.retranslateUI();
        ui->retranslateUi(this);
        setBackupsTitle();
    }

    SyncSettingsUIBase::changeEvent(event);
}

void BackupSettingsUI::reqRemoveSync(std::shared_ptr<SyncSettings> backup)
{
    removeSync(backup);
}

void BackupSettingsUI::removeSync(std::shared_ptr<SyncSettings> backup)
{
    QPointer<RemoveBackupDialog> dialog = new RemoveBackupDialog(backup, this);

    DialogOpener::showDialog(dialog,[this, dialog]()
    {
        if(dialog->result() == QDialog::Accepted)
        {
            syncsStateInformation(SyncStateInformation::SAVING);
            mSyncController->removeSync(dialog->backupToRemove(), dialog->targetFolder());
        }
    });
}

QString BackupSettingsUI::getFinishWarningIconString() const
{
#ifdef Q_OS_MACOS
    return QString::fromUtf8("settings-backups-error");
#else
    return QString::fromUtf8(":/images/settings-backups-warn.png");
#endif
}

QString BackupSettingsUI::getFinishIconString() const
{
#ifdef Q_OS_MACOS
    return QString::fromUtf8("settings-backup");
#else
    return QString::fromUtf8(":/images/settings-backup.png");
#endif
}

QString BackupSettingsUI::getOperationFailTitle() const
{
    return tr("Sync operation failed");
}

QString BackupSettingsUI::getOperationFailText(std::shared_ptr<SyncSettings> sync)
{
    return tr("Operation on sync '%1' failed. Reason: %2")
        .arg(sync->name(),
             QCoreApplication::translate("MegaSyncError", mega::MegaSync::getMegaSyncErrorCode(sync->getError())));
}

QString BackupSettingsUI::getErrorAddingTitle() const
{
    return tr("Error adding sync");
}

QString BackupSettingsUI::getErrorRemovingTitle()const
{
    return tr("Error removing backup");
}

QString BackupSettingsUI::getErrorRemovingText(std::shared_ptr<mega::MegaError> err)
{
    return tr("Your sync can't be removed. Reason: %1")
        .arg(QCoreApplication::translate("MegaError", err->getErrorString()));
}

void BackupSettingsUI::setBackupsTitle()
{
    setTitle(tr("Backups"));
}

QString BackupSettingsUI::disableString() const
{
    return tr("Some folders haven't been backed up. For more information, hover over the red icon.");
}
