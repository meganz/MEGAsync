#include "SyncSettingsUI.h"

#include "CreateRemoveSyncsManager.h"
#include "DialogOpener.h"
#include "MegaApplication.h"
#include "Onboarding.h"
#include "QmlDialogWrapper.h"
#include "SyncController.h"
#include "SyncItemModel.h"
#include "SyncsComponent.h"
#include "SyncTableView.h"

SyncSettingsUI::SyncSettingsUI(QWidget* parent):
    SyncSettingsUIBase(parent)
{
    setSyncsTitle();
    setTable<SyncTableView, SyncItemModel, SyncController>();

    mSyncElement.initElements(this);

    connect(MegaSyncApp,
            &MegaApplication::storageStateChanged,
            this,
            &SyncSettingsUI::storageStateChanged);
    storageStateChanged(MegaSyncApp->getAppliedStorageState());

    // There was a problem with the sync height on Windows with large scales
#ifdef Q_OS_WINDOWS
    adjustSize();
#endif

    if (auto dialog = DialogOpener::findDialog<QmlDialogWrapper<Onboarding>>())
    {
        setAddButtonEnabled(!dialog->getDialog()->isVisible());
        connect(dialog->getDialog(),
                &QmlDialogWrapper<Onboarding>::finished,
                this,
                [this]()
                {
                    setAddButtonEnabled(true);
                });
    }

    if (auto dialog = DialogOpener::findDialog<QmlDialogWrapper<SyncsComponent>>())
    {
        setAddButtonEnabled(!dialog->getDialog()->isVisible());
    }
}

void SyncSettingsUI::addButtonClicked(mega::MegaHandle megaFolderHandle)
{
    CreateRemoveSyncsManager::addSync(megaFolderHandle, true);
}

QString SyncSettingsUI::getFinishWarningIconString() const
{
    return QString::fromUtf8(":/images/settings-sync-warn.svg");
}

QString SyncSettingsUI::getFinishIconString() const
{
    return QString::fromUtf8(":/images/settings-sync.svg");
}

QString SyncSettingsUI::disableString() const
{
    return tr(
        "Some folders have not synchronised. For more information please hover over the red icon.");
}

QString SyncSettingsUI::getOperationFailTitle() const
{
    return tr("Sync operation failed");
}

QString SyncSettingsUI::getOperationFailText(std::shared_ptr<SyncSettings> sync)
{
    return tr("Operation on sync '%1' failed. Reason: %2")
        .arg(sync->name(),
             QCoreApplication::translate("MegaSyncError",
                                         mega::MegaSync::getMegaSyncErrorCode(sync->getError())));
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

void SyncSettingsUI::removeSync(std::shared_ptr<SyncSettings> sync)
{
    CreateRemoveSyncsManager::removeSync(sync, this);
}

void SyncSettingsUI::setSyncsTitle()
{
    setTitle(tr("Synced Folders"));
}

void SyncSettingsUI::changeEvent(QEvent* event)
{
    if (event->type() == QEvent::LanguageChange)
    {
        mSyncElement.retranslateUi();
        setSyncsTitle();
    }

    SyncSettingsUIBase::changeEvent(event);
}

void SyncSettingsUI::storageStateChanged(int newStorageState)
{
    mSyncElement.setOverQuotaMode(newStorageState == mega::MegaApi::STORAGE_STATE_RED ||
                                  newStorageState == mega::MegaApi::STORAGE_STATE_PAYWALL);
}
