#include "SyncSettingsUI.h"

#include "syncs/gui/Twoways/SyncTableView.h"
#include "syncs/model/SyncItemModel.h"
#include <MegaApplication.h>

SyncSettingsUI::SyncSettingsUI(QWidget *parent) :
    SyncSettingsUIBase(parent)
{
    setTitle(tr("Synced Folders"));
    setTable<SyncTableView,SyncItemModel>();

    mSyncElement.initElements(this);

    connect(MegaSyncApp, &MegaApplication::storageStateChanged, this, &SyncSettingsUI::storageStateChanged);
    storageStateChanged(MegaSyncApp->getAppliedStorageState());

    //There was a problem with the sync height on Windows with large scales
#ifdef Q_OS_WINDOWS
    adjustSize();
#endif
}

SyncSettingsUI::~SyncSettingsUI()
{
}

QString SyncSettingsUI::getFinishWarningIconString() const
{
#ifdef Q_OS_MACOS
    return QString::fromUtf8("settings-syncs-error");
#else
    return QString::fromUtf8(":/images/settings-syncs-warn.png");
#endif
}

QString SyncSettingsUI::getFinishIconString() const
{
#ifdef Q_OS_MACOS
    return QString::fromUtf8("settings-syncs");
#else
    return QString::fromUtf8(":/images/settings-syncs.png");
#endif
}


QString SyncSettingsUI::disableString() const
{
    return tr("Some folders have not synchronised. For more information please hover over the red icon.");
}

QString SyncSettingsUI::getOperationFailTitle() const
{
    return tr("Sync operation failed");
}

QString SyncSettingsUI::getOperationFailText(std::shared_ptr<SyncSettings> sync)
{
    return tr("Operation on sync '%1' failed. Reason: %2")
        .arg(sync->name(),
             QCoreApplication::translate("MegaSyncError", mega::MegaSync::getMegaSyncErrorCode(sync->getError())));
}

QString SyncSettingsUI::getErrorAddingTitle() const
{
    return tr("Error adding sync");
}

QString SyncSettingsUI::getErrorRemovingTitle() const
{
    return tr("Error removing backup");
}

QString SyncSettingsUI::getErrorRemovingText(std::shared_ptr<mega::MegaError> err)
{
    return tr("Your sync can't be removed. Reason: %1")
        .arg(QCoreApplication::translate("MegaError", err->getErrorString()));
}

void SyncSettingsUI::storageStateChanged(int newStorageState)
{
    mSyncElement.setOverQuotaMode(newStorageState == mega::MegaApi::STORAGE_STATE_RED
                                  || newStorageState == mega::MegaApi::STORAGE_STATE_PAYWALL);
}






