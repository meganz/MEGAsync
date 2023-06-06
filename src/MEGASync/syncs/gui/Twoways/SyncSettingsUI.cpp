#include "SyncSettingsUI.h"

#include "syncs/gui/Twoways/SyncTableView.h"
#include "syncs/model/SyncItemModel.h"

SyncSettingsUI::SyncSettingsUI(QWidget *parent) :
    SyncSettingsUIBase(parent)
{
    setType(mega::MegaSync::SyncType::TYPE_TWOWAY);
    setTable<SyncTableView,SyncItemModel>();

    mSyncElement.initElements(this);
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

void SyncSettingsUI::setOverQuotaMode(bool state)
{
    mSyncElement.setOverQuotaMode(state);
}






