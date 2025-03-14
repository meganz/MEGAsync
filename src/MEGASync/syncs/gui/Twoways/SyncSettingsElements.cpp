#include "SyncSettingsElements.h"

#ifdef Q_OS_MACOS
#include "CocoaHelpButton.h"
#endif
#include "MegaApplication.h"
#include "Preferences.h"
#include "StalledIssuesModel.h"
#include "SyncController.h"
#include "SyncInfo.h"
#include "SyncSettingsUIBase.h"
#include "TextDecorator.h"
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
                Utilities::openUrl(QUrl(Utilities::SYNC_SUPPORT_URL));
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

    // If user doesn´t have legacy rules, we don´t show the button as it won´t do anything
    syncStallModeSelectorUI->gBugReport->setVisible(
        Preferences::instance()->hasLegacyExclusionRules());

    connect(syncStallModeSelectorUI->bApplyLegacyExclusions,
            &QPushButton::clicked,
            this,
            &SyncSettingsElements::applyPreviousExclusions);

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

    emit MegaSyncApp->updateUserInterface();
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
    }
}

void SyncSettingsElements::onAdvanceModeSelected(bool checked)
{
    if (checked)
    {
        Preferences::instance()->setStalledIssuesMode(Preferences::StalledIssuesModeType::Advance);
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

void SyncSettingsElements::applyPreviousExclusions()
{
    MessageBoxInfo msgInfo;
    msgInfo.parent = mSyncStallModeSelector;
    msgInfo.titleText = tr("[B]Apply previous exclusion rules?[/B]");
    Text::Bold boldDecroator;
    boldDecroator.process(msgInfo.titleText);
    msgInfo.descriptionText =
        tr("The exclusion rules you set up in a previous version of the app will be applied to all "
           "of your syncs and backups. Any rules created since then will be overwritten.");
    msgInfo.textFormat = Qt::RichText;
    msgInfo.buttons = QMessageBox::Ok | QMessageBox::Cancel;
    QMap<QMessageBox::Button, QString> textsByButton;
    textsByButton.insert(QMessageBox::Ok, tr("Apply"));
    msgInfo.buttonsText = textsByButton;
    msgInfo.finishFunc = [](QPointer<MessageBoxResult> msg)
    {
        if (msg->result() == QMessageBox::Ok)
        {
            // Replace existing mega ignores files in all syncs
            SyncController::instance().resetAllSyncsMegaIgnoreUsingLegacyRules();
        }
    };
    QMegaMessageBox::warning(msgInfo);
}
