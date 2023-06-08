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
}

SyncSettingsUI::~SyncSettingsUI()
{
}

QString SyncSettingsUI::getFinishWarningIconString()
{
    return QString::fromUtf8(":/images/settings-syncs-warn.png");
}

QString SyncSettingsUI::getFinishIconString()
{
    return QString::fromUtf8(":/images/settings-syncs.png");
}

QString SyncSettingsUI::typeString()
{
    return tr("sync");
}

void SyncSettingsUI::storageStateChanged(int newStorageState)
{
    mSyncElement.setOverQuotaMode(newStorageState == mega::MegaApi::STORAGE_STATE_RED
                                  || newStorageState == mega::MegaApi::STORAGE_STATE_PAYWALL);
}






