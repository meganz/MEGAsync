#include "SyncSettingsQuickWidget.h"

#include "CreateRemoveSyncsManager.h"
#include "MessageDialogOpener.h"
#include "Preferences.h"
#include "QmlManager.h"
#include "RequestListenerManager.h"
#include "StalledIssuesModel.h"
#include "StatsEventHandler.h"
#include "SyncController.h"
#include "SyncSettingsModel.h"

SyncSettingsQuickWidget::SyncSettingsQuickWidget(QWidget* parent):
    SettingsQuickWidgetBase(new SyncSettingsModel(), SyncController::instance(), parent),
    mAutomaticSyncIssueResolverEnabled(Preferences::instance()->isStalledIssueSmartModeActivated())
{
    qmlRegisterType<SyncSettingsModel>("SyncSettingsModel", 1, 0, "SyncSettingsModel");

    QmlManager::instance()->setRootContextProperty(QStringLiteral("syncSettingsModel"), model());
    // Distinct context-property name: the Syncs and Backups widgets share one QML
    // engine root context, so they must not use the same name. The shared QML
    // components receive the widget as an explicit "settingsAccess" property instead.
    QmlManager::instance()->setRootContextProperty(QStringLiteral("syncSettingsAccess"), this);

    setSource(QString::fromUtf8("qrc:/settings/SyncSettings.qml"));
}

void SyncSettingsQuickWidget::addItem() const
{
    CreateRemoveSyncsManager::addSync(SyncInfo::SyncOrigin::SETTINGS_ORIGIN,
                                      mega::INVALID_HANDLE,
                                      QString(),
                                      this->window());
}

void SyncSettingsQuickWidget::remove(int index) const
{
    CreateRemoveSyncsManager::removeSync(model()->getSyncSetting(index), this->parentWidget());
}

bool SyncSettingsQuickWidget::getAutomaticSyncIssueResolverEnabled() const
{
    return mAutomaticSyncIssueResolverEnabled;
}

void SyncSettingsQuickWidget::restoreSyncedFolder(int index)
{
    auto sync = model()->getSyncSetting(index);

    auto triggerErrorMessage = [sync, this]()
    {
        QString logMsg = tr("Can't restore %1 mega folder").arg(sync->getMegaFolder());

        MessageDialogInfo msgInfo;
        msgInfo.parent = this->parentWidget();
        msgInfo.descriptionText = logMsg;
        msgInfo.buttons = QMessageBox::Cancel | QMessageBox::Yes;
        QMap<QMessageBox::Button, QString> textsByButton;
        textsByButton.insert(QMessageBox::Yes, tr("Remove sync"));
        textsByButton.insert(
            QMessageBox::Cancel,
            tr("Close")); // :-( , need to add this, so i've the outline style button.
        msgInfo.buttonsText = textsByButton;

        msgInfo.defaultButton = QMessageBox::Close;
        msgInfo.finishFunc = [sync](QPointer<MessageDialogResult> msg)
        {
            if (msg->result() == QMessageBox::Yes)
            {
                SyncController::instance().removeSync(sync);
            }
        };

        MessageDialogOpener::critical(msgInfo);

        mega::MegaApi::log(mega::MegaApi::LOG_LEVEL_ERROR, logMsg.toUtf8().constData());
    };

    auto node = std::shared_ptr<mega::MegaNode>(
        MegaSyncApp->getMegaApi()->getNodeByHandle(sync->getMegaHandle()));
    if (node)
    {
        auto restoreNode = std::shared_ptr<mega::MegaNode>(
            MegaSyncApp->getMegaApi()->getNodeByHandle(node->getRestoreHandle()));

        if (restoreNode)
        {
            auto listener = RequestListenerManager::instance().registerAndGetCustomFinishListener(
                this,
                [sync, triggerErrorMessage](mega::MegaRequest* request, mega::MegaError* e)
                {
                    int errorCode = e->getErrorCode();

                    if (errorCode != mega::MegaError::API_OK)
                    {
                        triggerErrorMessage();
                    }
                    else
                    {
                        SyncController::instance().setSyncToRun(sync);
                    }
                });

            MegaSyncApp->getMegaApi()->moveNode(node.get(), restoreNode.get(), listener.get());

            return;
        }
    }

    triggerErrorMessage();
}

void SyncSettingsQuickWidget::setAutomaticSyncIssueResolverEnabled(bool enable)
{
    if (mAutomaticSyncIssueResolverEnabled != enable)
    {
        if (enable)
        {
            Preferences::instance()->setStalledIssuesMode(
                Preferences::StalledIssuesModeType::Smart);
            MegaSyncApp->getStalledIssuesModel()->updateActiveStalledIssues();
            MegaSyncApp->getStatsEventHandler()->sendEvent(
                AppStatsEvents::EventType::SETTINGS_ISSUE_RESOLUTION_SMART);
        }
        else
        {
            Preferences::instance()->setStalledIssuesMode(
                Preferences::StalledIssuesModeType::Advance);
            MegaSyncApp->getStatsEventHandler()->sendEvent(
                AppStatsEvents::EventType::SETTINGS_ISSUE_RESOLUTION_ADVANCED);
        }

        mAutomaticSyncIssueResolverEnabled = enable;
        emit automaticSyncIssueResolverEnabledChanged();
    }
}
