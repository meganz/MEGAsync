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
#include <GuiUtilities.h>
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
        connect(table, &TableType::signaladdExclusions, this, &SyncSettingsUIBase::openExclusionsDialog);
        connect(table, &TableType::signalOpenMegaignore, this, &SyncSettingsUIBase::openMegaIgnore);
        connect(table, &TableType::signalRescanQuick, this, &SyncSettingsUIBase::rescanQuick);
        connect(table, &TableType::signalRescanDeep, this, &SyncSettingsUIBase::rescanDeep);

        auto& model = mModels[table->getType()];
        if(!model)
        {
            model = new ModelType(mTable);
            model->fillData();
            connect(model.data(), &ModelType::signalSyncCheckboxOn, this, &SyncSettingsUIBase::setSyncToRun);
            connect(model.data(), &ModelType::signalSyncCheckboxOff, this, &SyncSettingsUIBase::setSyncToSuspend);

            connect(model.data(), &ModelType::syncUpdateFinished, this, [this](std::shared_ptr<SyncSettings> syncSetting)
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
            QString messageBoxTitle(getOperationFailTitle());
            auto it = messageBoxTitle.begin();
            (*it) = it->toUpper();
            QMegaMessageBox::MessageBoxInfo msgInfo;
            msgInfo.parent = this;
            msgInfo.title = messageBoxTitle;
            msgInfo.text = getOperationFailText(sync);
            msgInfo.textFormat = Qt::RichText;
            QMegaMessageBox::critical(msgInfo);
        });

        connect(mSyncController, &SyncController::syncAddStatus, this, [this](int errorCode, int syncErrorCode, const QString errorMsg, const QString localPath)
        {
            const QString title = getErrorAddingTitle();

            if (Preferences::instance()->accountType() == mega::MegaAccountDetails::ACCOUNT_TYPE_PRO_FLEXI &&
                syncErrorCode == mega::MegaSync::ACCOUNT_EXPIRED)
            {
                QString message = tr("%1 can't be added as your Pro Flexi account has been deactivated due to payment failure "
                             "or you've cancelled your subscription. To continue, make a payment and reactivate your subscription.").arg(localPath);
                GuiUtilities::showPayReactivateOrDismiss(title, message);
            }
            else
            {
                if (errorCode != mega::MegaError::API_OK)
                {
                    onSavingSyncsCompleted(SAVING_FINISHED);
                    Text::Link link(QString::fromUtf8("https://mega.nz/contact"));
                    Text::Decorator dec(&link);
                    QString msg = errorMsg;
                    dec.process(msg);

                    QMegaMessageBox::MessageBoxInfo msgInfo;
                    msgInfo.parent = this;
                    msgInfo.title = title;
                    msgInfo.text = msg;
                    msgInfo.textFormat = Qt::RichText;
                    QMegaMessageBox::warning(msgInfo);
                }
            }
        });

        connect(mSyncController, &SyncController::syncRemoveError, this, [this](std::shared_ptr<mega::MegaError> err)
        {
            onSavingSyncsCompleted(SAVING_FINISHED);

            QMegaMessageBox::MessageBoxInfo msgInfo;
            msgInfo.parent = this;
            msgInfo.title = getErrorRemovingTitle();
            msgInfo.text =  getErrorRemovingText(err);
            msgInfo.textFormat = Qt::RichText;
            QMegaMessageBox::warning(msgInfo);
        });

        setDisabledSyncsText();
        syncsStateInformation(SAVING_FINISHED);
    }

#ifdef Q_OS_MACOS
    void setToolBarItem(QMacToolBarItem* item);
#else
    void setToolBarItem(QToolButton* item);
#endif

    template <class DialogType>
    void setParentDialog(DialogType *newParentDialog)
    {
        mParentDialog = newParentDialog;
        connect(this, &SyncSettingsUIBase::disableParentDialog, newParentDialog, &DialogType::setEnabledAllControls);
    }

public slots:
    virtual void addButtonClicked(mega::MegaHandle megaFolderHandle = mega::INVALID_HANDLE);
#ifndef Q_OS_WINDOWS
    void onPermissionsClicked();
#endif

signals:
    void disableParentDialog(bool state);

protected:
    Ui::SyncSettingsUIBase* ui;
    SyncTableView* mTable;
    SyncController* mSyncController;
    QDialog* mParentDialog;

    virtual QString getFinishWarningIconString() = 0;
    virtual QString getFinishIconString() = 0;
    virtual QString disableString() = 0;

    //Operation failed
    virtual QString getOperationFailTitle() = 0;
    virtual QString getOperationFailText(std::shared_ptr<SyncSettings> sync) = 0;

    //Error adding
    virtual QString getErrorAddingTitle() = 0;

    //Error removing
    virtual QString getErrorRemovingTitle() = 0;
    virtual QString getErrorRemovingText(std::shared_ptr<mega::MegaError> err) = 0;

protected slots:
    virtual void removeSyncButtonClicked();
    virtual void removeSync(std::shared_ptr<SyncSettings> sync);
    void setSyncToRun(std::shared_ptr<SyncSettings> sync);
    void setSyncToPause(std::shared_ptr<SyncSettings> sync);
    void setSyncToSuspend(std::shared_ptr<SyncSettings> sync);
    void setSyncToDisabled(std::shared_ptr<SyncSettings> sync);

    void rescanQuick(std::shared_ptr<SyncSettings>);
    void rescanDeep(std::shared_ptr<SyncSettings>);

    void openExclusionsDialog(std::shared_ptr<SyncSettings> sync);
    void openMegaIgnore(std::shared_ptr<SyncSettings>);
    void showOpenMegaIgnoreError();
    void onOpenMegaIgnoreFinished();

private:
    void initTable();
    void setModel(SyncItemModel* model);
    void addSyncFolderAfterOverQuotaCheck(mega::MegaHandle megaFolderHandle);
    void setDisabledSyncsText();

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
