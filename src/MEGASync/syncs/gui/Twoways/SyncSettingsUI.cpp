#include "SyncSettingsUI.h"

#include "syncs/gui/Twoways/SyncTableView.h"
#include "syncs/model/SyncItemModel.h"

SyncSettingsUI::SyncSettingsUI(QWidget *parent) :
    SyncSettingsUIBase(parent)
{
    setType(mega::MegaSync::SyncType::TYPE_TWOWAY);
    setTable<SyncTableView,SyncItemModel>();
}

SyncSettingsUI::~SyncSettingsUI()
{
}






