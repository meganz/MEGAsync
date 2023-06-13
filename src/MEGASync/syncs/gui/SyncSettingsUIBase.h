#ifndef SYNCSETTINGSUIBASE_H
#define SYNCSETTINGSUIBASE_H

#include <QWidget>
#include <QIcon>
#include <QFutureWatcher>
#ifdef Q_OS_MACOS
#include "platform/macx/QCustomMacToolbar.h"
#else
#include <QToolButton>
#endif

#include <QMegaMessageBox.h>
#include <TextDecorator.h>
#include <Utilities.h>
#include <MegaApplication.h>

namespace Ui {
class SyncSettingsUIBase;
}

class SyncItemModel;
class SyncItemSortModel;
class SyncController;
class SyncTableView;

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

    void setTitle(const QString& title);

    void insertUIElement(QWidget* widget, int position);

    void onSavingSyncsCompleted(SyncStateInformation value);
    void syncsStateInformation(SyncStateInformation state);

    template <class TableType, class ModelType, class SortModelType = SyncItemSortModel>
    void setTable()
    {
        auto table = new TableType();
        mTable = table;
        initTable();

        connect(table, &TableType::signalRunSync, this, &SyncSettingsUIBase::setSyncToRun);
        connect(table, &TableType::signalPauseSync, this, &SyncSettingsUIBase::setSyncToPause);
        connect(table, &TableType::signalSuspendSync, this, &SyncSettingsUIBase::setSyncToSuspend);
        connect(table, &TableType::signalDisableSync, this, &SyncSettingsUIBase::setSyncToDisabled);
        connect(table, &TableType::signalRemoveSync, this, &SyncSettingsUIBase::removeSync);
        connect(table, &TableType::signalOpenMegaignore, this, &SyncSettingsUIBase::openMegaIgnore);
        connect(table, &TableType::signalRescanQuick, this, &SyncSettingsUIBase::rescanQuick);
        connect(table, &TableType::signalRescanDeep, this, &SyncSettingsUIBase::rescanDeep);

        auto& model = mModels[mTable->getType()];
        if(!model)
        {
            model = new ModelType(mTable);
            model->fillData();
            connect(model, &ModelType::signalSyncCheckboxOn, this, &SyncSettingsUIBase::setSyncToRun);
            connect(model, &ModelType::signalSyncCheckboxOff, this, &SyncSettingsUIBase::setSyncToSuspend);

            connect(model, &ModelType::syncUpdateFinished, this, [this](std::shared_ptr<SyncSettings> syncSetting)
            {
                onSavingSyncsCompleted(SAVING_FINISHED);
            });
        }

        SortModelType *sortModel = new SortModelType(mTable);
        sortModel->setSourceModel(model);
        table->setModel(sortModel);

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

        syncsStateInformation(SAVING_FINISHED);
    }

#ifdef Q_OS_MACOS
    void setToolBarItem(QMacToolBarItem* item);
#else
    void setToolBarItem(QToolButton* item);
#endif

    void setParentDialog(QDialog *newParentDialog);

public slots:
    virtual void addButtonClicked(mega::MegaHandle megaFolderHandle = mega::INVALID_HANDLE);
#ifndef Q_OS_WINDOWS
    void onPermissionsClicked();
#endif

protected:
    Ui::SyncSettingsUIBase* ui;
    SyncTableView* mTable;
    SyncController* mSyncController;
    QDialog* mParentDialog;

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
    static QMap<mega::MegaSync::SyncType,QPointer<SyncItemModel>> mModels;
    QFutureWatcher<bool> mOpeMegaIgnoreWatcher;

#ifdef Q_OS_MACOS
    QMacToolBarItem* mToolBarItem;
#else
    QToolButton* mToolBarItem;
#endif
};

#endif // SYNCSETTINGSUIBASE_H
