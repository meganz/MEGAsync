#include "SyncSettingsElements.h"
#include "ui_SyncAccountFullMessage.h"
#include "ui_SyncStallModeSelector.h"

#include <syncs/gui/SyncSettingsUIBase.h>

#include <Utilities.h>
#include "control/Preferences/Preferences.h"
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

    syncSettingsUi->insertUIElement(syncAccountFull, 1);

    QWidget* syncStallModeSelector(new QWidget());
    syncStallModeSelectorUI->setupUi(syncStallModeSelector);
    syncStallModeSelectorUI->LearnMoreButton->setAutoDefault(false);

    auto mode = Preferences::instance()->stalledIssuesMode();
    if(mode == Preferences::StalledIssuesModeType::Smart)
    {
        syncStallModeSelectorUI->SmartSelector->setChecked(true);
    }
    //Lazy hack -> if no mode has been selected, the Advance one is "selected" on the GUI (not on preferences)
    else
    {
        syncStallModeSelectorUI->AdvanceSelector->setChecked(true);
    }

    connect(syncStallModeSelectorUI->LearnMoreButton, &QPushButton::clicked,[](){
        Utilities::openUrl(QUrl(Utilities::SYNC_SUPPORT_URL));
    });
    connect(syncStallModeSelectorUI->SmartSelector, &QRadioButton::toggled, this, &SyncSettingsElements::onSmartModeSelected);
    connect(syncStallModeSelectorUI->AdvanceSelector, &QRadioButton::toggled, this, &SyncSettingsElements::onAdvanceModeSelected);

    connect(Preferences::instance().get(), &Preferences::valueChanged, this, &SyncSettingsElements::onPreferencesValueChanged);

    syncSettingsUi->insertUIElement(syncStallModeSelector, 2);
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

void SyncSettingsElements::onPreferencesValueChanged(QString key)
{
    if(key == Preferences::stalledIssuesModeKey)
    {
        auto modeSelected = Preferences::instance()->stalledIssuesMode();

        if(modeSelected == Preferences::StalledIssuesModeType::Smart && !syncStallModeSelectorUI->SmartSelector->isChecked())
        {
            syncStallModeSelectorUI->SmartSelector->blockSignals(true);
            syncStallModeSelectorUI->SmartSelector->setChecked(true);
            syncStallModeSelectorUI->SmartSelector->blockSignals(false);
        }
        else if(modeSelected == Preferences::StalledIssuesModeType::Advance && !syncStallModeSelectorUI->AdvanceSelector->isChecked())
        {
            syncStallModeSelectorUI->AdvanceSelector->blockSignals(true);
            syncStallModeSelectorUI->AdvanceSelector->setChecked(true);
            syncStallModeSelectorUI->AdvanceSelector->blockSignals(false);
        }
    }
}

