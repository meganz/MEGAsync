#ifndef SYNCSETTINGSUIBASE_H
#define SYNCSETTINGSUIBASE_H

#include "GuiUtilities.h"
#include "Preferences.h"
#include "QMegaMessageBox.h"
#include "SyncInfo.h"
#include "SyncSettings.h"
#include "TextDecorator.h"

#include <QFutureWatcher>
#include <QIcon>
#include <QToolButton>
#include <QWidget>

namespace Ui
{
class SyncSettingsUIBase;
}

class SyncItemModel;
class SyncItemSortModel;
class SyncTableView;

class SyncSettingsUIBase: public QWidget
{
    Q_OBJECT

public:
    SyncSettingsUIBase(QWidget* parent);

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

    void setAddButtonEnabled(bool enabled);

    template<class TableType,
             class ModelType,
             class Controller,
             class SortModelType = SyncItemSortModel>
    void setTable()
    {
        syncsStateInformation(SAVING);

        auto table = new TableType();
        mTable = table;
        initTable();

        connect(table,
                &TableType::signalRunSync,
                this,
                [](std::shared_ptr<SyncSettings> sync)
                {
                    Controller::instance().setSyncToRun(sync);
                });

        connect(table,
                &TableType::signalSuspendSync,
                this,
                [](std::shared_ptr<SyncSettings> sync)
                {
                    Controller::instance().setSyncToSuspend(sync);
                });

        connect(table, &TableType::signalRemoveSync, this, &SyncSettingsUIBase::removeSync);
        connect(table,
                &TableType::signaladdExclusions,
                this,
                &SyncSettingsUIBase::openExclusionsDialog);
        connect(table, &TableType::signalOpenMegaignore, this, &SyncSettingsUIBase::openMegaIgnore);
        connect(table, &TableType::signalRescanQuick, this, &SyncSettingsUIBase::rescanQuick);
        connect(table, &TableType::signalRescanDeep, this, &SyncSettingsUIBase::rescanDeep);
        connect(table, &TableType::signalReboot, this, &SyncSettingsUIBase::reboot);

        auto& model = mModels[table->getType()];
        if (!model)
        {
            model = new ModelType(mTable);
            model->fillData();
            connect(model.data(),
                    &ModelType::signalSyncCheckboxOn,
                    this,
                    [](std::shared_ptr<SyncSettings> sync)
                    {
                        Controller::instance().setSyncToRun(sync);
                    });
            connect(model.data(),
                    &ModelType::signalSyncCheckboxOff,
                    this,
                    [](std::shared_ptr<SyncSettings> sync)
                    {
                        Controller::instance().setSyncToSuspend(sync);
                    });
        }

        SortModelType* sortModel = new SortModelType(mTable);
        sortModel->setSourceModel(model);
        table->setModel(sortModel);

        connect(&Controller::instance(),
                &Controller::signalSyncOperationBegins,
                this,
                [this]()
                {
                    syncsStateInformation(SyncStateInformation::SAVING);
                });

        connect(&Controller::instance(),
                &Controller::signalSyncOperationEnds,
                this,
                [this]()
                {
                    onSavingSyncsCompleted(SyncStateInformation::SAVING_FINISHED);
                });

        connect(&Controller::instance(),
                &Controller::signalSyncOperationError,
                this,
                [this](std::shared_ptr<SyncSettings> sync)
                {
                    QString messageBoxTitle(getOperationFailTitle());
                    auto it = messageBoxTitle.begin();
                    (*it) = it->toUpper();
                    MessageBoxInfo msgInfo;
                    msgInfo.parent = this;
                    msgInfo.dialogTitle = messageBoxTitle;
                    msgInfo.titleText = getOperationFailText(sync);
                    msgInfo.textFormat = Qt::RichText;
                    QMegaMessageBox::critical(msgInfo);
                });

        connect(&Controller::instance(),
                &Controller::syncAddStatus,
                this,
                [this](int errorCode, int syncErrorCode, const QString localPath)
                {
                    const QString title = getErrorAddingTitle();

                    if (Preferences::instance()->accountType() ==
                            mega::MegaAccountDetails::ACCOUNT_TYPE_PRO_FLEXI &&
                        syncErrorCode == mega::MegaSync::ACCOUNT_EXPIRED)
                    {
                        Text::Bold bold;
                        Text::Decorator dec(&bold);

                        QString message = tr("%1 can't be added as your Pro Flexi account has been "
                                             "deactivated due to payment failure "
                                             "or you've cancelled your subscription. To continue, "
                                             "make a payment and reactivate your subscription.")
                                              .arg(localPath);
                        dec.process(message);
                        GuiUtilities::showPayReactivateOrDismiss(title, message);
                    }
                    else
                    {
                        if (errorCode != mega::MegaError::API_OK)
                        {
                            Text::Link link(QString::fromUtf8("https://mega.nz/contact"));
                            Text::Decorator dec(&link);
                            QString msg =
                                Controller::instance().getErrorString(errorCode, syncErrorCode);
                            dec.process(msg);

                            MessageBoxInfo msgInfo;
                            msgInfo.parent = this;
                            msgInfo.dialogTitle = title;
                            msgInfo.titleText = msg;
                            msgInfo.textFormat = Qt::RichText;
                            QMegaMessageBox::warning(msgInfo);
                        }
                    }
                });

        connect(&Controller::instance(),
                &Controller::syncRemoveError,
                this,
                [this](std::shared_ptr<mega::MegaError> err)
                {
                    MessageBoxInfo msgInfo;
                    msgInfo.parent = this;
                    msgInfo.dialogTitle = getErrorRemovingTitle();
                    msgInfo.titleText = getErrorRemovingText(err);
                    msgInfo.textFormat = Qt::RichText;
                    QMegaMessageBox::warning(msgInfo);
                });

        setDisabledSyncsText();
        syncsStateInformation(SAVING_FINISHED);
    }

    void setToolBarItem(QToolButton* item);

    template<class DialogType>
    void setParentDialog(DialogType* newParentDialog)
    {
        mParentDialog = newParentDialog;
        connect(this,
                &SyncSettingsUIBase::disableParentDialog,
                newParentDialog,
                &DialogType::setEnabledAllControls);
    }

public slots:
    virtual void addButtonClicked(mega::MegaHandle = mega::INVALID_HANDLE) = 0;
#ifndef Q_OS_WINDOWS
    void onPermissionsClicked();
#endif

signals:
    void disableParentDialog(bool state);

protected:
    Ui::SyncSettingsUIBase* ui;
    SyncTableView* mTable;
    QDialog* mParentDialog;

    virtual QString getFinishWarningIconString() const = 0;
    virtual QString getFinishIconString() const = 0;
    virtual QString disableString() const = 0;

    // Operation failed
    virtual QString getOperationFailTitle() const = 0;
    virtual QString getOperationFailText(std::shared_ptr<SyncSettings> sync) = 0;

    // Error adding
    virtual QString getErrorAddingTitle() const = 0;

    // Error removing
    virtual QString getErrorRemovingTitle() const = 0;
    virtual QString getErrorRemovingText(std::shared_ptr<mega::MegaError> err) = 0;

protected slots:
    virtual void removeSync(std::shared_ptr<SyncSettings> sync) = 0;

    void rescanQuick(std::shared_ptr<SyncSettings>);
    void rescanDeep(std::shared_ptr<SyncSettings>);
    void reboot(std::shared_ptr<SyncSettings>);

    void openExclusionsDialog(std::shared_ptr<SyncSettings> sync);
    void openMegaIgnore(std::shared_ptr<SyncSettings>);
    void showOpenMegaIgnoreError();
    void onOpenMegaIgnoreFinished();

private:
    void initTable();
    void setDisabledSyncsText();

    SyncInfo* mSyncInfo;
    static QMap<mega::MegaSync::SyncType, QPointer<SyncItemModel>> mModels;
    QFutureWatcher<bool> mOpeMegaIgnoreWatcher;
    QToolButton* mToolBarItem;
};

#endif // SYNCSETTINGSUIBASE_H
