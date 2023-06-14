#include "BackupSettingsElements.h"
#include "ui_OpenBackupsFolder.h"

#include <syncs/gui/SyncSettingsUIBase.h>
#include "UserAttributesRequests/MyBackupsHandle.h"

BackupSettingsElements::BackupSettingsElements(QObject *parent) :
    QObject(parent),
    openFolderUi(new Ui::OpenBackupsFolder)
{
}

BackupSettingsElements::~BackupSettingsElements()
{
    delete openFolderUi;
}

void BackupSettingsElements::initElements(SyncSettingsUIBase *syncSettingsUi)
{
    QWidget* openBackupsFolder(new QWidget());
    openFolderUi->setupUi(openBackupsFolder);
    openFolderUi->bOpenBackupFolder->setEnabled(false);
    openFolderUi->bOpenBackupFolder->setAutoDefault(false);
    connect(openFolderUi->bOpenBackupFolder, &QPushButton::clicked, this, &BackupSettingsElements::onOpenBackupFolderClicked);

    auto myBackupsHandle = UserAttributes::MyBackupsHandle::requestMyBackupsHandle();
    connect(myBackupsHandle.get(), &UserAttributes::MyBackupsHandle::attributeReady,
            this, &BackupSettingsElements::onMyBackupsFolderHandleSet);
    onMyBackupsFolderHandleSet(myBackupsHandle->getMyBackupsHandle());

    syncSettingsUi->insertUIElement(openBackupsFolder, 1);
}

void BackupSettingsElements::updateUI()
{
    QString backupsDirPath = UserAttributes::MyBackupsHandle::getMyBackupsLocalizedPath();
    openFolderUi->lBackupFolder->setText(backupsDirPath);
}

void BackupSettingsElements::onOpenBackupFolderClicked()
{
    auto myBackupsHandle = UserAttributes::MyBackupsHandle::requestMyBackupsHandle();
    Utilities::openInMega(myBackupsHandle->getMyBackupsHandle());
}

void BackupSettingsElements::onMyBackupsFolderHandleSet(mega::MegaHandle h)
{
    updateUI();

    if (h == mega::INVALID_HANDLE)
    {
        openFolderUi->bOpenBackupFolder->setEnabled(false);
    }
    else
    {
        openFolderUi->bOpenBackupFolder->setEnabled(true);
    }
}
