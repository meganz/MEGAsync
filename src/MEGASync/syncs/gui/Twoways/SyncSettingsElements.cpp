#include "SyncSettingsElements.h"
#include "ui_SyncAccountFullMessage.h"
#include "ui_SyncStallModeSelector.h"

#include <syncs/gui/SyncSettingsUIBase.h>

#include <Utilities.h>
#include <Preferences.h>
#include <MegaApplication.h>
#include <StalledIssuesModel.h>

SyncSettingsElements::SyncSettingsElements(QObject *parent) :
    QObject(parent),
    syncAccountFullMessageUI(new Ui::SyncAccountFullMessage),
    syncStallModeSelectorUI(new Ui::SyncStallModeSelector)
{
}

SyncSettingsElements::~SyncSettingsElements()
{
    delete syncAccountFullMessageUI;
}


void SyncSettingsElements::initElements(SyncSettingsUIBase* syncSettingsUi)
{
    QWidget* syncAccountFull(new QWidget());
    syncAccountFullMessageUI->setupUi(syncAccountFull);
    connect(syncAccountFullMessageUI->bBuyMoreSpace, &QPushButton::clicked, this, &SyncSettingsElements::onPurchaseMoreStorage);

    syncSettingsUi->insertUIElement(syncAccountFull, 0);

    QWidget* syncStallModeSelector(new QWidget());
    syncStallModeSelectorUI->setupUi(syncStallModeSelector);

    auto mode = Preferences::instance()->stalledIssuesMode();
    if(mode != Preferences::StalledIssuesModeType::None)
    {
        if(mode == Preferences::StalledIssuesModeType::Smart)
        {
           syncStallModeSelectorUI->SmartSelector->setChecked(true);
        }
        else
        {
            syncStallModeSelectorUI->AdvanceSelector->setChecked(true);
        }
    }

    connect(syncStallModeSelectorUI->SmartSelector, &QRadioButton::toggled, this, &SyncSettingsElements::onSmartModeSelected);
    connect(syncStallModeSelectorUI->AdvanceSelector, &QRadioButton::toggled, this, &SyncSettingsElements::onAdvanceModeSelected);

    syncSettingsUi->insertUIElement(syncStallModeSelector, 1);
}

void SyncSettingsElements::setOverQuotaMode(bool mode)
{
    syncAccountFullMessageUI->wOQError->setVisible(mode);
}

void SyncSettingsElements::onPurchaseMoreStorage()
{
    Utilities::upgradeClicked();
}

void SyncSettingsElements::onSmartModeSelected(bool checked)
{
    if(checked)
    {
        Preferences::instance()->setStalledIssuesMode(Preferences::StalledIssuesModeType::Smart);
        //Update the model to fix automatically the issues
        MegaSyncApp->getStalledIssuesModel()->updateStalledIssues();
    }
}

void SyncSettingsElements::onAdvanceModeSelected(bool checked)
{
    if(checked)
    {
        Preferences::instance()->setStalledIssuesMode(Preferences::StalledIssuesModeType::Advance);
    }
}

