#include "SyncSettingsActions.h"

#include "CreateRemoveSyncsManager.h"
#include "MegaApplication.h"
#include "MessageDialogOpener.h"
#include "Preferences.h"
#include "RequestListenerManager.h"
#include "StalledIssuesModel.h"
#include "StatsEventHandler.h"
#include "SyncController.h"
#include "SyncSettingsModel.h"

#include <QCoreApplication>

SyncSettingsActions::SyncSettingsActions(QWidget* hostWidget, QObject* parent):
    SyncSettingsActionsBase(new SyncSettingsModel(),
                            SyncController::instance(),
                            hostWidget,
                            parent),
    mAutomaticSyncIssueResolverEnabled(Preferences::instance()->isStalledIssueSmartModeActivated())
{}

void SyncSettingsActions::addItem() const
{
    CreateRemoveSyncsManager::addSync(SyncInfo::SyncOrigin::SETTINGS_ORIGIN,
                                      mega::INVALID_HANDLE,
                                      QString(),
                                      hostWindow());
}

void SyncSettingsActions::remove(int index) const
{
    CreateRemoveSyncsManager::removeSync(model()->getSyncSetting(index), hostParentWidget());
}

bool SyncSettingsActions::getAutomaticSyncIssueResolverEnabled() const
{
    return mAutomaticSyncIssueResolverEnabled;
}

void SyncSettingsActions::restoreSyncedFolder(int index)
{
    auto sync = model()->getSyncSetting(index);

    auto triggerErrorMessage = [sync, this]()
    {
        // Translation context kept from SyncSettingsQuickWidget (pre SNC-6795) so
        // the existing translations still apply.
        QString logMsg =
            QCoreApplication::translate("SyncSettingsQuickWidget", "Can't restore %1 mega folder")
                .arg(sync->getMegaFolder());

        MessageDialogInfo msgInfo;
        msgInfo.parent = hostParentWidget();
        msgInfo.descriptionText = logMsg;
        msgInfo.buttons = QMessageBox::Cancel | QMessageBox::Yes;
        QMap<QMessageBox::Button, QString> textsByButton;
        textsByButton.insert(QMessageBox::Yes,
                             QCoreApplication::translate("SyncSettingsQuickWidget", "Remove sync"));
        // Cancel is needed so the dialog gets the outline style button.
        textsByButton.insert(QMessageBox::Cancel,
                             QCoreApplication::translate("SyncSettingsQuickWidget", "Close"));
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
        if (MegaSyncApp->getMegaApi()->isInRubbish(node.get()))
        {
            auto restoreNode = std::shared_ptr<mega::MegaNode>(
                MegaSyncApp->getMegaApi()->getNodeByHandle(node->getRestoreHandle()));

            if (restoreNode)
            {
                auto listener =
                    RequestListenerManager::instance().registerAndGetCustomFinishListener(
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
        else // node is back to it's original place.
        {
            SyncController::instance().setSyncToRun(sync);

            return;
        }
    }

    triggerErrorMessage();
}

void SyncSettingsActions::setAutomaticSyncIssueResolverEnabled(bool enable)
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
