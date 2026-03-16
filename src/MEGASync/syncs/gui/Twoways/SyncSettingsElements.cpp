#include "SyncSettingsElements.h"

#include "MegaApplication.h"
#include "Preferences.h"
#include "ServiceUrls.h"
#include "StalledIssuesModel.h"
#include "StatsEventHandler.h"
#include "SyncController.h"
#include "SyncInfo.h"
#include "SyncSettingsUIBase.h"
#include "ui_SyncAccountFullMessage.h"
#include "ui_SyncStallModeSelector.h"
#include "Utilities.h"

SyncSettingsElements::SyncSettingsElements(QObject* parent):
    QObject(parent),
    syncAccountFullMessageUI(new Ui::SyncAccountFullMessage),
    syncStallModeSelectorUI(new Ui::SyncStallModeSelector),
    mSyncAccountFull(nullptr),
    mSyncStallModeSelector(nullptr)
{}

SyncSettingsElements::~SyncSettingsElements()
{
    delete syncAccountFullMessageUI;
}

void SyncSettingsElements::initElements(SyncSettingsUIBase* syncSettingsUi)
{
    mSyncAccountFull = new QWidget();
    syncAccountFullMessageUI->setupUi(mSyncAccountFull);
    connect(syncAccountFullMessageUI->bBuyMoreSpace,
            &QPushButton::clicked,
            this,
            &SyncSettingsElements::onPurchaseMoreStorage);

    syncSettingsUi->insertUIElement(mSyncAccountFull, 1);

    mSyncStallModeSelector = new QWidget();
    syncStallModeSelectorUI->setupUi(mSyncStallModeSelector);

    connect(syncStallModeSelectorUI->LearnMoreButton,
            &QPushButton::clicked,
            []()
            {
                Utilities::openUrl(ServiceUrls::getSyncHelpUrl());
            });

    auto mode = Preferences::instance()->stalledIssuesMode();
    if (mode == Preferences::StalledIssuesModeType::Smart)
    {
        syncStallModeSelectorUI->SmartSelector->setChecked(true);
    }
    // Lazy hack -> if no mode has been selected, the Advance one is "selected" on the GUI (not on
    // preferences)
    else
    {
        syncStallModeSelectorUI->AdvanceSelector->setChecked(true);
    }

    connect(syncStallModeSelectorUI->SmartSelector,
            &QRadioButton::toggled,
            this,
            &SyncSettingsElements::onSmartModeSelected);

    connect(syncStallModeSelectorUI->AdvanceSelector,
            &QRadioButton::toggled,
            this,
            &SyncSettingsElements::onAdvanceModeSelected);

    connect(Preferences::instance().get(),
            &Preferences::valueChanged,
            this,
            &SyncSettingsElements::onPreferencesValueChanged);

    syncSettingsUi->insertUIElement(mSyncStallModeSelector, 2);
}

void SyncSettingsElements::setOverQuotaMode(bool mode)
{
    syncAccountFullMessageUI->wOQError->setVisible(mode);
}

void SyncSettingsElements::retranslateUi()
{
    syncAccountFullMessageUI->retranslateUi(mSyncAccountFull);
    syncStallModeSelectorUI->retranslateUi(mSyncStallModeSelector);
}

void SyncSettingsElements::onPurchaseMoreStorage()
{
    Utilities::upgradeClicked();
}

void SyncSettingsElements::onSmartModeSelected(bool checked)
{
    if (checked)
    {
        Preferences::instance()->setStalledIssuesMode(Preferences::StalledIssuesModeType::Smart);
        // Update the model to fix automatically the issues
        MegaSyncApp->getStalledIssuesModel()->updateActiveStalledIssues();
        MegaSyncApp->getStatsEventHandler()->sendEvent(
            AppStatsEvents::EventType::SETTINGS_ISSUE_RESOLUTION_SMART);
    }
}

void SyncSettingsElements::onAdvanceModeSelected(bool checked)
{
    if (checked)
    {
        Preferences::instance()->setStalledIssuesMode(Preferences::StalledIssuesModeType::Advance);
        MegaSyncApp->getStatsEventHandler()->sendEvent(
            AppStatsEvents::EventType::SETTINGS_ISSUE_RESOLUTION_ADVANCED);
    }
}

void SyncSettingsElements::onPreferencesValueChanged(QString key)
{
    if (key == Preferences::stalledIssuesModeKey)
    {
        auto modeSelected = Preferences::instance()->stalledIssuesMode();

        if (modeSelected == Preferences::StalledIssuesModeType::Smart &&
            !syncStallModeSelectorUI->SmartSelector->isChecked())
        {
            syncStallModeSelectorUI->SmartSelector->blockSignals(true);
            syncStallModeSelectorUI->SmartSelector->setChecked(true);
            syncStallModeSelectorUI->SmartSelector->blockSignals(false);
        }
        else if (modeSelected == Preferences::StalledIssuesModeType::Advance &&
                 !syncStallModeSelectorUI->AdvanceSelector->isChecked())
        {
            syncStallModeSelectorUI->AdvanceSelector->blockSignals(true);
            syncStallModeSelectorUI->AdvanceSelector->setChecked(true);
            syncStallModeSelectorUI->AdvanceSelector->blockSignals(false);
        }
    }
}
