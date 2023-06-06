#include "SyncSettingsUIBase.h"

#include <QDateTime>

#include <DialogOpener.h>

#include <syncs/gui/Twoways/SyncTableView.h>
#include <syncs/model/SyncItemModel.h>

#include "ui_SyncSettingsUIBase.h"

SyncSettingsUIBase::SyncSettingsUIBase(QWidget *parent):
    QWidget(parent),
    ui(new Ui::SyncSettingsUIBase),
    mTable(nullptr),
    mSyncController(new SyncController(this)),
    mSyncInfo(SyncInfo::instance()),
    mModel(nullptr),
    mType(mega::MegaSync::SyncType::TYPE_TWOWAY)
{
    ui->setupUi(this);

    connect(&mOpeMegaIgnoreWatcher, &QFutureWatcher<bool>::finished, this, &SyncSettingsUIBase::onOpenMegaIgnoreFinished);

    connect(ui->gSyncs, &RemoteItemUi::addClicked, this, &SyncSettingsUIBase::addButtonClicked);
    connect(ui->gSyncs, &RemoteItemUi::deleteClicked, this, &SyncSettingsUIBase::removeSyncButtonClicked);
}

void SyncSettingsUIBase::insertUIElement(QWidget *widget, int position)
{
    ui->SyncSettingsLayout->insertWidget(position, widget);

    if(position == 0 && ui->SyncSettingsLayout->count() > 0)
    {
        auto secondWidget = ui->SyncSettingsLayout->itemAt(position +1)->widget();
        setTabOrder(widget,secondWidget);
    }
    else if(position > 0 && position < ui->SyncSettingsLayout->count())
    {
        auto previousWidget = ui->SyncSettingsLayout->itemAt(position-1)->widget();
        setTabOrder(previousWidget,widget);
        auto nextWidget = ui->SyncSettingsLayout->itemAt(position + 1)->widget();
        setTabOrder(widget, nextWidget);
    }
    else if(position > 0  && position >= ui->SyncSettingsLayout->count())
    {
        auto previousWidget = ui->SyncSettingsLayout->itemAt(position-1)->widget();
        setTabOrder(previousWidget,widget);
    }
}

void SyncSettingsUIBase::onSavingSyncsCompleted(SyncStateInformation value)
{
    qint64 startTime(0);
    if(value == SyncStateInformation::SAVING_FINISHED)
    {
        startTime =  ui->wSpinningIndicatorSyncs->getStartTime();
    }
    auto closeDelay = std::max(0ll, 350ll - (QDateTime::currentMSecsSinceEpoch()
                                             - startTime));
    QTimer::singleShot(closeDelay, this, [this, value] () {
        syncsStateInformation(value);
    });
}

void SyncSettingsUIBase::syncsStateInformation(SyncStateInformation state)
{
        switch (state)
        {
            case SAVING:
                emit enableStateChanged(false);
                ui->wSpinningIndicatorSyncs->start();
                ui->sSyncsState->setCurrentWidget(ui->pSavingSyncs);
                break;
            case SAVING_FINISHED:
                emit enableStateChanged(true);
                ui->wSpinningIndicatorSyncs->stop();
                // If any sync is disabled, shows warning message
                if (mSyncInfo->syncWithErrorExist(mType))
                {
                    ui->sSyncsState->setCurrentWidget(ui->pSyncsDisabled);

    #ifdef Q_OS_MACOS
                    QCustomMacToolbar* toolBar = dynamic_cast<QCustomMacToolbar*>(mToolBarItem->parent());
                    if(toolBar)
                    {
                        toolBar->customizeIconToolBarItem(mToolBarItem, getFinishWarningIconString());
                    }
    #else
                    mToolBarItem->setIcon(QIcon(getFinishWarningIconString()));
    #endif
                }
                else
                {
                    ui->sSyncsState->setCurrentWidget(ui->pNoErrorsSyncs);

    #ifdef Q_OS_MACOS
                    QCustomMacToolbar* toolBar = dynamic_cast<QCustomMacToolbar*>(mToolBarItem->parent());
                    if(toolBar)
                    {
                        toolBar->customizeIconToolBarItem(mToolBarItem, getFinishIconString());
                    }
    #else
                    mToolBarItem->setIcon(QIcon(getFinishIconString()));
    #endif
                }
                break;
        }
}

#ifdef Q_OS_MACOS
#else
void SyncSettingsUIBase::setToolBarItem(QToolButton *item)
{
    mToolBarItem = item;

    syncsStateInformation(SAVING_FINISHED);
}
#endif

void SyncSettingsUIBase::addButtonClicked(mega::MegaHandle megaFolderHandle)
{
    auto overQuotaDialog = MegaSyncApp->showSyncOverquotaDialog();
    if(overQuotaDialog)
    {
        DialogOpener::showDialog(overQuotaDialog, [megaFolderHandle, overQuotaDialog, this]()
        {
            if(overQuotaDialog->result() == QDialog::Rejected)
            {
                addSyncFolderAfterOverQuotaCheck(megaFolderHandle);
            }
        });
    }
    else
    {
        addSyncFolderAfterOverQuotaCheck(megaFolderHandle);
    }
}

void SyncSettingsUIBase::addSyncFolderAfterOverQuotaCheck(mega::MegaHandle megaFolderHandle)
{
    QPointer<BindFolderDialog> dialog = new BindFolderDialog(MegaSyncApp, this);

    if (megaFolderHandle != mega::INVALID_HANDLE)
    {
        dialog->setMegaFolder(megaFolderHandle);
    }

    DialogOpener::showDialog<BindFolderDialog>(dialog, [dialog, this]()
    {
        QString localFolderPath = QDir::toNativeSeparators(QDir(dialog->getLocalFolder())
                                                           .canonicalPath());

    if (localFolderPath.isEmpty() || dialog->getMegaPath().isEmpty()
        || dialog->getSyncName().isEmpty() || !dialog->getMegaFolder())
    {
        return;
    }

    syncsStateInformation(SyncStateInformation::SAVING);
    mSyncController->addSync(localFolderPath, dialog->getMegaFolder(), dialog->getSyncName(), mega::MegaSync::TYPE_TWOWAY);
    });

}

void SyncSettingsUIBase::removeSyncButtonClicked()
{
    if(mTable->selectionModel()->hasSelection())
    {
        QModelIndex index = mTable->selectionModel()->selectedRows().first();
        removeSync(index.data(Qt::UserRole).value<std::shared_ptr<SyncSettings>>());
    }
}

void SyncSettingsUIBase::removeSync(std::shared_ptr<SyncSettings> sync)
{
    syncsStateInformation(SAVING);
    mSyncController->removeSync(sync);
}

void SyncSettingsUIBase::setSyncToRun(std::shared_ptr<SyncSettings> sync)
{
    syncsStateInformation(SAVING);
    mSyncController->setSyncToRun(sync);
}

void SyncSettingsUIBase::setSyncToPause(std::shared_ptr<SyncSettings> sync)
{
    syncsStateInformation(SAVING);
    mSyncController->setSyncToPause(sync);
}

void SyncSettingsUIBase::setSyncToSuspend(std::shared_ptr<SyncSettings> sync)
{
    syncsStateInformation(SAVING);
    mSyncController->setSyncToSuspend(sync);
}

void SyncSettingsUIBase::setSyncToDisabled(std::shared_ptr<SyncSettings> sync)
{
    syncsStateInformation(SAVING);
    mSyncController->setSyncToDisabled(sync);
}

void SyncSettingsUIBase::openMegaIgnore(std::shared_ptr<SyncSettings> sync)
{
    QString ignore(sync->getLocalFolder() + QDir::separator() + QString::fromUtf8(".megaignore"));
    auto future = QtConcurrent::run(QDesktopServices::openUrl, QUrl::fromLocalFile(ignore));
    mOpeMegaIgnoreWatcher.setFuture(future);
}

void SyncSettingsUIBase::onOpenMegaIgnoreFinished()
{
    auto result = mOpeMegaIgnoreWatcher.result();
    if(!result)
    {
        showOpenMegaIgnoreError();
    }
}

void SyncSettingsUIBase::initTable()
{
    ui->gSyncs->initView(mTable);
}

void SyncSettingsUIBase::showOpenMegaIgnoreError()
{
    QMegaMessageBox::warning(nullptr, tr("Error"), tr("Error opening megaignore file"), QMessageBox::Ok);
}

void SyncSettingsUIBase::rescanQuick(std::shared_ptr<SyncSettings> sync)
{
    MegaSyncApp->getMegaApi()->rescanSync(sync->backupId(), false);
}

void SyncSettingsUIBase::rescanDeep(std::shared_ptr<SyncSettings> sync)
{
     MegaSyncApp->getMegaApi()->rescanSync(sync->backupId(), true);
}

