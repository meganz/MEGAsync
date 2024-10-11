#include "SyncSettingsElements.h"

#ifdef Q_OS_MACOS
#include "CocoaHelpButton.h"
#endif
#include "Preferences.h"
#include "SyncInfo.h"
#include "SyncSettingsUIBase.h"
#include "TextDecorator.h"
#include "ui_SyncAccountFullMessage.h"
#include "ui_SyncStallModeSelector.h"

#include <MegaApplication.h>
#include <StalledIssuesModel.h>
#include <Utilities.h>

constexpr char MEGA_IGNORE_FILE_NAME[] = ".megaignore";
constexpr char MEGA_IGNORE_DEFAULT_FILE_NAME[] = ".megaignore.default";

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

#ifdef Q_OS_MACOS
    CocoaHelpButton* LearnMoreButton = new CocoaHelpButton();
    syncStallModeSelectorUI->horizontalLayout_3->insertWidget(1, LearnMoreButton);
    syncStallModeSelectorUI->bApplyLegacyExclusions->setAutoDefault(false);
    connect(LearnMoreButton,
            &CocoaHelpButton::clicked,
            []()
            {
                Utilities::openUrl(QUrl(Utilities::SYNC_SUPPORT_URL));
            });
#else
    connect(syncStallModeSelectorUI->LearnMoreButton,
            &QPushButton::clicked,
            []()
            {
                Utilities::openUrl(QUrl(Utilities::SYNC_SUPPORT_URL));
            });
#endif
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

    //! TODO Josep Subirana Oller: FIX!!!
    // connect(syncStallModeSelectorUI->bApplyLegacyExclusions,
    //         &QPushButton::clicked,
    //         this,
    //         &SyncSettingsElements::applyPreviousExclusions);
    //! END TODO Josep Subirana Oller: FIX!!!
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
    QMegaMessageBox::MessageBoxInfo msgInfo;
    msgInfo.parent = mSyncStallModeSelector;
    msgInfo.text = tr("[B]Apply previous exclusion rules?[/B]");
    Text::Bold boldDecroator;
    boldDecroator.process(msgInfo.text);
    msgInfo.informativeText =
        tr("The exclusion rules you set up in a previous version of the app will be applied to all "
           "of your syncs and backups. Any rules created since then will be overwritten.");
    msgInfo.textFormat = Qt::RichText;
    msgInfo.buttons = QMessageBox::Ok | QMessageBox::Cancel;
    QMap<QMessageBox::Button, QString> textsByButton;
    textsByButton.insert(QMessageBox::Ok, tr("Apply"));
    msgInfo.buttonsText = textsByButton;
    msgInfo.finishFunc = [](QPointer<QMessageBox> msg)
    {
        if (msg->result() == QMessageBox::Ok)
        {
            // Step 0: Remove old default ignore
            const auto defaultIgnoreFolder = Preferences::instance()->getDataPath();
            const auto defaultIgnorePath = defaultIgnoreFolder + QString::fromUtf8("/") +
                                           QString::fromUtf8(::MEGA_IGNORE_DEFAULT_FILE_NAME);
            QFile::remove(defaultIgnorePath);
            // Step 1: Replace default ignore with one populated with legacy rules
            MegaSyncApp->getMegaApi()->exportLegacyExclusionRules(
                defaultIgnoreFolder.toStdString().c_str());
            QFile::rename(defaultIgnoreFolder + QString::fromUtf8("/") +
                              QString::fromUtf8(::MEGA_IGNORE_FILE_NAME),
                          defaultIgnorePath);
            // Step 2: Replace existing mega ignores files in all syncs
            const auto syncsSettings = SyncInfo::instance()->getAllSyncSettings();
            for (auto sync: syncsSettings)
            {
                QFile ignoreFile(sync->getLocalFolder() + QString::fromUtf8("/") +
                                 QString::fromUtf8(::MEGA_IGNORE_FILE_NAME));
                if (ignoreFile.exists())
                {
                    ignoreFile.moveToTrash();
                }
                MegaSyncApp->getMegaApi()->exportLegacyExclusionRules(
                    sync->getLocalFolder().toStdString().c_str());
            }
        }
    };
    QMegaMessageBox::warning(msgInfo);
}
