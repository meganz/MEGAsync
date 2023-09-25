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

QString SyncSettingsUI::getFinishWarningIconString()
{
#ifdef Q_OS_MACOS
    return QString::fromUtf8("settings-syncs-error");
#else
    return QString::fromUtf8(":/images/settings-syncs-warn.png");
#endif
}

QString SyncSettingsUI::getFinishIconString()
{
#ifdef Q_OS_MACOS
    return QString::fromUtf8("settings-syncs");
#else
    return QString::fromUtf8(":/images/settings-syncs.png");
#endif
}

QString SyncSettingsUI::typeString()
{
    return tr("sync");
}

QString SyncSettingsUI::disableString()
{
    return tr("Some folders have not synchronised. For more information please hover over the red icon.");
}

void SyncSettingsUI::storageStateChanged(int newStorageState)
{
    mSyncElement.setOverQuotaMode(newStorageState == mega::MegaApi::STORAGE_STATE_RED
                                  || newStorageState == mega::MegaApi::STORAGE_STATE_PAYWALL);
}






