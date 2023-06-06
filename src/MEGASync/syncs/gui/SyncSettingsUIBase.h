#ifndef SYNCSETTINGSUIBASE_H
#define SYNCSETTINGSUIBASE_H

#include <QWidget>
#include <QIcon>
#include <QFutureWatcher>
#ifdef Q_OS_MACOS
#include <QCustomMacToolbar.h>
#else
#include <QToolButton>
#endif

#include <syncs/control/SyncInfo.h>
#include <syncs/model/SyncItemModel.h>
#include <QMegaMessageBox.h>
#include <TextDecorator.h>
#include <Utilities.h>
#include <MegaApplication.h>

namespace Ui {
class SyncSettingsUIBase;
}

class SyncTableView;
class SyncItemModel;
class SyncController;

class SyncSettingsUIBase : public QWidget
{
    Q_OBJECT

public:
    SyncSettingsUIBase(QWidget *parent);

    virtual ~SyncSettingsUIBase() = default;

    enum SyncStateInformation
    {
        SAVING = 0,
        SAVING_FINISHED,
    };

    void onSavingSyncsCompleted(SyncStateInformation value);
    void syncsStateInformation(SyncStateInformation state);

    void setOverQuotaMode(bool mode);

    template <class TableType, class ModelType, class SortModelType = SyncItemSortModel>
    void setTable()
    {
        mTable = new TableType();
        initTable();

        connect(mTable, &SyncTableView::signalRunSync, this, &SyncSettingsUIBase::setSyncToRun);
        connect(mTable, &SyncTableView::signalPauseSync, this, &SyncSettingsUIBase::setSyncToPause);
        connect(mTable, &SyncTableView::signalSuspendSync, this, &SyncSettingsUIBase::setSyncToSuspend);
        connect(mTable, &SyncTableView::signalDisableSync, this, &SyncSettingsUIBase::setSyncToDisabled);
        connect(mTable, &SyncTableView::signalRemoveSync, this, &SyncSettingsUIBase::removeSync);
        connect(mTable, &SyncTableView::signalOpenMegaignore, this, &SyncSettingsUIBase::openMegaIgnore);
        connect(mTable, &SyncTableView::signalRescanQuick, this, &SyncSettingsUIBase::rescanQuick);
        connect(mTable, &SyncTableView::signalRescanDeep, this, &SyncSettingsUIBase::rescanDeep);

        mModel = new ModelType(mTable);
        mModel->fillData();
        connect(mModel, &SyncItemModel::signalSyncCheckboxOn, this, &SyncSettingsUIBase::setSyncToRun);
        connect(mModel, &SyncItemModel::signalSyncCheckboxOff, this, &SyncSettingsUIBase::setSyncToSuspend);

        connect(mModel, &SyncItemModel::syncUpdateFinished, this, [this](std::shared_ptr<SyncSettings> syncSetting)
        {
                onSavingSyncsCompleted(SAVING_FINISHED);
        });


        SortModelType *sortModel = new SortModelType(mTable);
        sortModel->setSourceModel(mModel);
        mTable->setModel(sortModel);

        connect(mSyncController, &SyncController::signalSyncOperationBegins, this, [this](std::shared_ptr<SyncSettings> sync)
        {
            syncsStateInformation(SyncStateInformation::SAVING);
        });

        connect(mSyncController, &SyncController::signalSyncOperationEnds, this, [this](std::shared_ptr<SyncSettings> sync)
        {
            onSavingSyncsCompleted(SyncStateInformation::SAVING_FINISHED);
        });

        connect(mSyncController, &SyncController::signalSyncOperationError, this, [this](std::shared_ptr<SyncSettings> sync)
        {
                QMegaMessageBox::critical(nullptr, tr("%1 operation failed"),
                                          tr("Operation on %1 '%2' failed. Reason: %3")
                                          .arg(typeString(), sync->name(), QCoreApplication::translate("MegaSyncError", mega::MegaSync::getMegaSyncErrorCode(sync->getError()))));
        });

        connect(mSyncController, &SyncController::syncAddStatus, this, [this](int errorCode, const QString errorMsg)
        {
            if (errorCode != mega::MegaError::API_OK)
            {
                onSavingSyncsCompleted(SAVING_FINISHED);
                Text::Link link(Utilities::SUPPORT_URL);
                Text::Decorator dec(&link);
                QString msg = errorMsg;
                dec.process(msg);
                QMegaMessageBox::warning(nullptr, tr("Error adding %1").arg(typeString()), msg, QMessageBox::Ok, QMessageBox::NoButton, QMap<QMessageBox::StandardButton, QString>(), Qt::RichText);
            }
        });

        connect(mSyncController, &SyncController::syncRemoveError, this, [this](std::shared_ptr<mega::MegaError> err)
        {
            onSavingSyncsCompleted(SAVING_FINISHED);
            QMegaMessageBox::warning(nullptr, tr("Error removing %1").arg(typeString()),
                                      tr("Your %1 can't be removed. Reason: %2")
                                      .arg(typeString(), QCoreApplication::translate("MegaError", err->getErrorString())));
        });
    }

    inline void setType(mega::MegaSync::SyncType newType)
    {
        mType = newType;
    }

#ifdef Q_OS_MACOS
    void setToolBarItem(QMacToolBarItem* item);
#else
    void setToolBarItem(QToolButton* item);
#endif

public slots:
    virtual void addButtonClicked(mega::MegaHandle megaFolderHandle = mega::INVALID_HANDLE);

signals:
    void enableStateChanged(bool state);

protected:
    Ui::SyncSettingsUIBase* ui;
    SyncTableView* mTable;
    SyncController* mSyncController;

    virtual QString getFinishWarningIconString(){return QString();}
    virtual QString getFinishIconString(){return QString();}
    virtual QString typeString(){return QString();}

protected slots:
    virtual void removeSyncButtonClicked();
    virtual void removeSync(std::shared_ptr<SyncSettings> sync);
    void setSyncToRun(std::shared_ptr<SyncSettings> sync);
    void setSyncToPause(std::shared_ptr<SyncSettings> sync);
    void setSyncToSuspend(std::shared_ptr<SyncSettings> sync);
    void setSyncToDisabled(std::shared_ptr<SyncSettings> sync);

    void rescanQuick(std::shared_ptr<SyncSettings>);
    void rescanDeep(std::shared_ptr<SyncSettings>);

    void openMegaIgnore(std::shared_ptr<SyncSettings>);
    void showOpenMegaIgnoreError();
    void onOpenMegaIgnoreFinished();

private:
    void initTable();
    void setModel(SyncItemModel* model);
    void addSyncFolderAfterOverQuotaCheck(mega::MegaHandle megaFolderHandle);

    SyncInfo* mSyncInfo;
    SyncItemModel* mModel;
    mega::MegaSync::SyncType mType;
    QFutureWatcher<bool> mOpeMegaIgnoreWatcher;

#ifdef Q_OS_MACOS
    QMacToolBarItem* mToolBarItem;
#else
    QToolButton* mToolBarItem;
#endif
};

#endif // SYNCSETTINGSUIBASE_H
