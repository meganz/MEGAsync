#include "NodeSelectorSpecializations.h"

#include "DialogOpener.h"
#include "DuplicatedNodeDialog.h"
#include "megaapi.h"
#include "MegaNodeNames.h"
#include "NodeSelectorModel.h"
#include "NodeSelectorProxyModel.h"
#include "NodeSelectorTreeViewWidgetSpecializations.h"
#include "Preferences.h"
#include "QMegaMessageBox.h"
#include "RequestListenerManager.h"
#include "StatsEventHandler.h"
#include "SyncInfo.h"
#include "TextDecorator.h"
#include "ui_NodeSelector.h"
#include "UploadToMegaDialog.h"

#include <QMessageBox>
#include <QPointer>

UploadNodeSelector::UploadNodeSelector(QWidget* parent):
    NodeSelector(SelectTypeSPtr(new UploadType), parent)
{
}

void UploadNodeSelector::createSpecialisedWidgets()
{
    ui->fBackups->hide();
    addCloudDrive();
    mCloudDriveWidget->setShowEmptyView(false);
    addIncomingShares();
}

void UploadNodeSelector::onOkButtonClicked()
{
    auto node = getSelectedNode();
    if(node)
    {
        int access = Utilities::getNodeAccess(node->getHandle());
        if (access < mega::MegaShare::ACCESS_READWRITE)
        {
            QMegaMessageBox::MessageBoxInfo msgInfo;
            msgInfo.parent = this;
            msgInfo.title = QMegaMessageBox::errorTitle();
            msgInfo.text = tr("You need Read & Write or Full access rights to be able to upload to the selected folder.");
            msgInfo.finishFunc = [this](QPointer<QMessageBox> msg){
                reject();
            };
            QMegaMessageBox::warning(msgInfo);
        }
        else
        {
            accept();
        }
    }
    else
    {
        showNotFoundNodeMessageBox();
    }
}

/////////////////////////////////////////////////////////////
DownloadNodeSelector::DownloadNodeSelector(QWidget* parent):
    NodeSelector(SelectTypeSPtr(new DownloadType), parent)
{
    setWindowTitle(tr("Download"));

}

void DownloadNodeSelector::createSpecialisedWidgets()
{
    addCloudDrive();
    addIncomingShares();
    addBackups();
}

void DownloadNodeSelector::onOkButtonClicked()
{
    QList<mega::MegaHandle> nodes = getMultiSelectionNodeHandle();
    int wrongNodes(0);
    foreach(auto& nodeHandle, nodes)
    {
        auto node = std::unique_ptr<mega::MegaNode>(mMegaApi->getNodeByHandle(nodeHandle));
        if(!node)
        {
            ++wrongNodes;
        }
    }

    QMegaMessageBox::MessageBoxInfo msgInfo;
    msgInfo.parent = this;
    msgInfo.title = QMegaMessageBox::errorTitle();
    msgInfo.finishFunc = [this](QPointer<QMessageBox> msg){
        reject();
    };

    if(wrongNodes == nodes.size())
    {
        if(ui->stackedWidget->currentIndex() == CLOUD_DRIVE)
        {
            msgInfo.text = tr("The item you selected has been removed. To reselect, close this window and try again.", "", wrongNodes);
            QMegaMessageBox::warning(msgInfo);
        }
        else
        {
            msgInfo.text = tr("You no longer have access to this item. Ask the owner to share again.", "", wrongNodes);
            QMegaMessageBox::warning(msgInfo);
        }
    }
    else if(wrongNodes > 0)
    {
        QString warningMsg1 = tr("%1 item selected", "", nodes.size()).arg(nodes.size());
        msgInfo.text = tr("%1. %2 has been removed. To reselect, close this window and try again.", "", wrongNodes).arg(warningMsg1).arg(wrongNodes);
        QMegaMessageBox::warning(msgInfo);
    }
    else
    {
        accept();
    }
}

/////////////////////////////////////////////////////////////
SyncNodeSelector::SyncNodeSelector(QWidget* parent):
    NodeSelector(SelectTypeSPtr(new SyncType), parent)
{
    if (isFullSync())
    {
        ui->fCloudDrive->setVisible(false);
        emit ui->bShowIncomingShares->clicked();
    }
}

void SyncNodeSelector::createSpecialisedWidgets()
{
    ui->fBackups->hide();
    addCloudDrive();
    mCloudDriveWidget->setShowEmptyView(false);
    addIncomingShares();
}


bool SyncNodeSelector::isFullSync()
{
    auto syncsList = SyncInfo::instance()->getSyncSettingsByType(SyncInfo::SyncType::TYPE_TWOWAY);
    auto foundIt =
        std::find_if(syncsList.cbegin(),
                     syncsList.cend(),
                     [](const auto& sync)
                     {
                         return (sync->getMegaFolder() == QLatin1String("/") && sync->isActive());
                     });

    return foundIt != syncsList.cend();
}

void SyncNodeSelector::onOkButtonClicked()
{
    auto node = getSelectedNode();
    if(node)
    {
        QMegaMessageBox::MessageBoxInfo msgInfo;
        msgInfo.parent = this;
        msgInfo.title = QMegaMessageBox::errorTitle();
        msgInfo.finishFunc = [this](QPointer<QMessageBox> msg){
            reject();
        };

        int access = Utilities::getNodeAccess(node->getHandle());
        if (access < mega::MegaShare::ACCESS_FULL)
        {
            msgInfo.text = tr("You need Full access right to be able to sync the selected folder.");
            QMegaMessageBox::warning(msgInfo);
        }
        else
        {
            std::unique_ptr<char[]>path(mMegaApi->getNodePath(node.get()));
            auto check = std::unique_ptr<mega::MegaNode>(mMegaApi->getNodeByPath(path.get()));
            if (!check)
            {
                msgInfo.text = tr("Invalid folder for synchronization.\n"
                                  "Please, ensure that you don't use characters like '\\' '/' or ':' in your folder names.");
                QMegaMessageBox::warning(msgInfo);
            }
            else
            {
                accept();
            }
        }
    }
    else
    {
        showNotFoundNodeMessageBox();
    }
}

/////////////////////////////////////////////////////////////
StreamNodeSelector::StreamNodeSelector(QWidget* parent):
    NodeSelector(SelectTypeSPtr(new StreamType), parent)
{}

void StreamNodeSelector::createSpecialisedWidgets()
{
    addCloudDrive();
    addIncomingShares();
    addBackups();
}

void StreamNodeSelector::onOkButtonClicked()
{
    auto node = getSelectedNode();
    if(node)
    {
        if (node->isFolder())
        {
            QMegaMessageBox::MessageBoxInfo msgInfo;
            msgInfo.parent = this;
            msgInfo.title = QMegaMessageBox::errorTitle();
            msgInfo.text = tr("Only files can be used for streaming.");
            msgInfo.finishFunc = [this](QPointer<QMessageBox>)
            {
                reject();
            };
            QMegaMessageBox::warning(msgInfo);
        }
        else
        {
            accept();
        }
    }
    else
    {
        showNotFoundNodeMessageBox();
    }
}

/////////////////////////////////////////////////////////////
CloudDriveNodeSelector::CloudDriveNodeSelector(QWidget* parent):
    NodeSelector(SelectTypeSPtr(new CloudDriveType), parent)
{
    setWindowTitle(MegaNodeNames::getCloudDriveName());

    mDragBackDrop = new QWidget(this);
    mDragBackDrop->hide();

    ui->fRubbish->show();
    resize(1280,800);
    setMinimumSize(700, 400);
    setAcceptDrops(true);

#ifndef Q_OS_MACOS
    Qt::WindowFlags flags = Qt::Window;
    this->setWindowFlags(flags);
#ifdef Q_OS_LINUX
    this->setWindowFlags(this->windowFlags());
#endif
#endif

    // Update last time opened
    Preferences::instance()->cloudDriveDialogOpened();

    // Send stats in case we didn´t send them in the current hour
    sendStats();
}

void CloudDriveNodeSelector::createSpecialisedWidgets()
{
    addCloudDrive();
    addIncomingShares();
    addBackups();
    addRubbish();

    enableDragAndDrop(true);
}

void CloudDriveNodeSelector::enableDragAndDrop(bool enable)
{
    mCloudDriveWidget->enableDragAndDrop(enable);
    mRubbishWidget->enableDragAndDrop(true);
    mIncomingSharesWidget->enableDragAndDrop(true);
}

void CloudDriveNodeSelector::sendStats()
{
    auto cloudDriveLastDateTimeStatSent(
        Preferences::instance()->cloudDriveDialogLastDateTimeStatSent());
    auto cloudDriveLastDateTimeOpened(
        Preferences::instance()->cloudDriveDialogLastDateTimeOpened());

    // No event sent during the current hour
    if (Utilities::hourHasChanged(cloudDriveLastDateTimeStatSent,
                                  QDateTime::currentDateTime().toSecsSinceEpoch()))
    {
        auto sendEvent = []()
        {
            MegaSyncApp->getStatsEventHandler()->sendEvent(
                AppStatsEvents::EventType::CLOUD_DRIVE_HOURLY_ACTIVE_USERS);

            Preferences::instance()->updateCloudDriveDialogLastDateTimeStatSent();
        };

        // If the CloudDrive is currently open, send the event
        // If the CloudDrive is not currently open but it was opened during the current hour, send
        // the event
        auto cloudDriveDialog(DialogOpener::findDialog<NodeSelector>());
        auto isCurrentlyOpen(cloudDriveDialog && dynamic_cast<CloudDriveNodeSelector*>(
                                                     cloudDriveDialog->getDialog().data()));

        if (isCurrentlyOpen)
        {
            sendEvent();
        }
        else
        {
            // If the hour between now and the last date time opened has changed, it is not the
            // current hour
            auto openedDuringCurrentHour(
                !(Utilities::hourHasChanged(QDateTime::currentDateTime().toSecsSinceEpoch(),
                                            cloudDriveLastDateTimeOpened)));
            if (openedDuringCurrentHour)
            {
                sendEvent();
            }
        }
    }
}

void CloudDriveNodeSelector::onCustomBottomButtonClicked(uint id)
{
    if(id == CloudDriveType::Upload)
    {
        auto selectedNode = getSelectedNode();
        if(selectedNode)
        {
            MegaSyncApp->runUploadActionWithTargetHandle(selectedNode->getHandle(), this);
        }
        else
        {
            showNotFoundNodeMessageBox();
        }
    }
    else if(id == CloudDriveType::Download)
    {
        MegaSyncApp->downloadACtionClickedWithHandles(getMultiSelectionNodeHandle());
    }
    else if(id == CloudDriveType::ClearRubbish)
    {
        QMegaMessageBox::MessageBoxInfo msgInfo;
        msgInfo.parent = this;
        msgInfo.title = QMegaMessageBox::errorTitle();
        msgInfo.buttons = QMessageBox::Yes | QMessageBox::No;
        QMap<QMessageBox::Button, QString> textsByButton;
        textsByButton.insert(QMessageBox::Yes, tr("Empty"));
        textsByButton.insert(QMessageBox::No, tr("Cancel"));
        msgInfo.buttonsText = textsByButton;
        msgInfo.text = tr("Empty Rubbish bin?");
        Text::Bold bold;
        Text::Decorator dec(&bold);
        msgInfo.informativeText = tr("All items will be permanently deleted. This action can [B]not[/B] be undone");
        dec.process(msgInfo.informativeText);
        msgInfo.finishFunc = [this](QPointer<QMessageBox> msg)
        {
            if (msg->result() == QMessageBox::Yes)
            {
                mRubbishWidget->setLoadingSceneVisible(true);
                MegaSyncApp->getMegaApi()->cleanRubbishBin();
            }
        };
        QMegaMessageBox::warning(msgInfo);
    }
}

void CloudDriveNodeSelector::onItemsAboutToBeMoved(const QList<mega::MegaHandle>& handles, int type)
{
    checkMovingItems(handles, type, NodeSelector::IncreaseOrDecrease::INCREASE);
}

void CloudDriveNodeSelector::onItemsAboutToBeMovedFailed(const QList<mega::MegaHandle>& handles,
                                                         int type)
{
    checkMovingItems(handles, type, NodeSelector::IncreaseOrDecrease::DECREASE);
}

void CloudDriveNodeSelector::onItemsAboutToBeRestored(const QSet<mega::MegaHandle>& handles)
{
    auto tabsInfo(getTabs(handles.values()));

    if (!tabsInfo.cloudDriveNodes.isEmpty())
    {
        mCloudDriveWidget->setParentOfRestoredNodes(handles);
    }

    if (!tabsInfo.incomingSharedNodes.isEmpty())
    {
        mIncomingSharesWidget->setParentOfRestoredNodes(handles);
    }
}

void CloudDriveNodeSelector::onItemAboutToBeReplaced(mega::MegaHandle handle)
{
    auto tabsInfo(getTabs(QList<mega::MegaHandle>() << handle));

    if (!tabsInfo.cloudDriveNodes.isEmpty())
    {
        mCloudDriveWidget->addHandleToBeReplaced(handle);
    }

    if (!tabsInfo.incomingSharedNodes.isEmpty())
    {
        mIncomingSharesWidget->addHandleToBeReplaced(handle);
    }
}

void CloudDriveNodeSelector::onItemsAboutToBeMerged(
    const QList<std::shared_ptr<NodeSelectorMergeInfo>>& mergesInfo,
    int actionType)
{
    performMergeAction(mergesInfo, actionType, NodeSelector::IncreaseOrDecrease::INCREASE);
}

void CloudDriveNodeSelector::onItemsAboutToBeMergedFailed(
    const QList<std::shared_ptr<NodeSelectorMergeInfo>>& mergesInfo,
    int actionType)
{
    performMergeAction(mergesInfo, actionType, NodeSelector::IncreaseOrDecrease::DECREASE);
}

void CloudDriveNodeSelector::performMergeAction(
    const QList<std::shared_ptr<NodeSelectorMergeInfo>>& mergesInfo,
    int actionType,
    IncreaseOrDecrease type)
{
    QList<NodeSelectorTreeViewWidget::SourceHandle> sourceHandles;
    QList<NodeSelectorTreeViewWidget::TargetHandle> targetHandles;
    for (const auto& info: mergesInfo)
    {
        sourceHandles.append(info->nodeToMerge->getHandle());
        targetHandles.append(info->nodeTarget->getHandle());
    }

    if (actionType != MoveActionType::COPY)
    {
        performItemsToBeMoved(sourceHandles,
                              NodeSelector::IncreaseOrDecrease::INCREASE,
                              true,
                              false);
    }

    auto fillMergeFolders =
        [type,
         &mergesInfo](const QList<NodeSelectorTreeViewWidget::TargetHandle>& targetHandlesByTab,
                      NodeSelectorTreeViewWidget* wid)
    {
        QMultiHash<NodeSelectorTreeViewWidget::SourceHandle,
                   NodeSelectorTreeViewWidget::TargetHandle>
            merges;
        for (const auto& info: mergesInfo)
        {
            if (targetHandlesByTab.contains(info->nodeTarget->getHandle()))
            {
                merges.insert(info->nodeToMerge->getHandle(), info->nodeTarget->getHandle());
            }
        }

        if (type == NodeSelector::IncreaseOrDecrease::INCREASE)
        {
            wid->increaseMovingNodes(merges.size());
            wid->setMergeFolderHandles(merges);
        }
        else
        {
            wid->decreaseMovingNodes(merges.size());
            wid->resetMergeFolderHandles(merges);
        }
    };

    auto targetTabsInfo(getTabs(targetHandles));

    if (!targetTabsInfo.cloudDriveNodes.isEmpty())
    {
        if (actionType != MoveActionType::COPY)
        {
            fillMergeFolders(targetTabsInfo.cloudDriveNodes, mCloudDriveWidget);
        }
        else
        {
            mCloudDriveWidget->setAsyncSelectedNodeHandle(targetTabsInfo.cloudDriveNodes);
            mCloudDriveWidget->selectPendingIndexes();
        }

        if (type == NodeSelector::IncreaseOrDecrease::INCREASE &&
            actionType == MoveActionType::RESTORE)
        {
            // Check with the handle if we are in CD or Incoming
            if (ui->stackedWidget->currentWidget() == mRubbishWidget)
            {
                onbShowCloudDriveClicked();
            }
        }
    }

    if (!targetTabsInfo.incomingSharedNodes.isEmpty())
    {
        if (actionType != MoveActionType::COPY)
        {
            fillMergeFolders(targetTabsInfo.incomingSharedNodes, mIncomingSharesWidget);
        }
        else
        {
            mIncomingSharesWidget->setAsyncSelectedNodeHandle(targetTabsInfo.incomingSharedNodes);
            mIncomingSharesWidget->selectPendingIndexes();
        }

        if (type == NodeSelector::IncreaseOrDecrease::INCREASE &&
            actionType == MoveActionType::RESTORE)
        {
            // Check with the handle if we are in CD or Incoming
            if (ui->stackedWidget->currentWidget() == mRubbishWidget)
            {
                onbShowIncomingSharesClicked();
            }
        }
    }
}

void CloudDriveNodeSelector::onOkButtonClicked()
{
    onCustomBottomButtonClicked(CloudDriveType::Download);
}

void CloudDriveNodeSelector::checkMovingItems(const QList<mega::MegaHandle>& handles,
                                              int moveType,
                                              NodeSelector::IncreaseOrDecrease type)
{
    if (moveType == MoveActionType::RESTORE)
    {
        auto tabsInfo(getTabs(handles));

        if (!tabsInfo.cloudDriveNodes.isEmpty())
        {
            type == NodeSelector::IncreaseOrDecrease::INCREASE ?
                mCloudDriveWidget->increaseMovingNodes(tabsInfo.cloudDriveNodes.size()) :
                mIncomingSharesWidget->decreaseMovingNodes(tabsInfo.cloudDriveNodes.size());
        }

        if (!tabsInfo.incomingSharedNodes.isEmpty())
        {
            type == NodeSelector::IncreaseOrDecrease::INCREASE ?
                mIncomingSharesWidget->increaseMovingNodes(tabsInfo.incomingSharedNodes.size()) :
                mIncomingSharesWidget->decreaseMovingNodes(tabsInfo.incomingSharedNodes.size());
        }

        selectTabs(tabsInfo);

        performItemsToBeMoved(handles, type, true, false);
    }
    else if (moveType == MoveActionType::DELETE_RUBBISH)
    {
        if (type == NodeSelector::IncreaseOrDecrease::INCREASE)
        {
            mRubbishWidget->increaseMovingNodes(handles.size());
        }
        else
        {
            mRubbishWidget->decreaseMovingNodes(handles.size());
        }

        performItemsToBeMoved(handles, type, true, false);
    }
    else if (moveType == MoveActionType::DELETE_PERMANENTLY)
    {
        performItemsToBeMoved(handles, type, true, false);
    }
    else if (moveType == MoveActionType::COPY)
    {
        performItemsToBeMoved(handles, type, false, true);
    }
    else
    {
        performItemsToBeMoved(handles, type, true, true);
    }
}

CloudDriveNodeSelector::HandlesByTab
    CloudDriveNodeSelector::getTabs(const QList<mega::MegaHandle>& handles)
{
    HandlesByTab info;

    if (!handles.isEmpty())
    {
        for (const auto& handle: handles)
        {
            std::unique_ptr<mega::MegaNode> node(
                MegaSyncApp->getMegaApi()->getNodeByHandle(handle));
            if (node)
            {
                mega::MegaNode* checkNode(nullptr);

                std::unique_ptr<mega::MegaNode> restoreNode(
                    MegaSyncApp->getMegaApi()->getNodeByHandle(node->getRestoreHandle()));
                if (restoreNode)
                {
                    checkNode = restoreNode.get();
                }
                else
                {
                    checkNode = node.get();
                }

                if (MegaSyncApp->getMegaApi()->isInCloud(checkNode))
                {
                    info.cloudDriveNodes.append(handle);
                }
                else
                {
                    info.incomingSharedNodes.append(handle);
                }
            }
        }
    }

    return info;
}

void CloudDriveNodeSelector::selectTabs(const HandlesByTab& tabsInfo)
{
    if (!tabsInfo.cloudDriveNodes.isEmpty())
    {
        onbShowCloudDriveClicked();
    }
    else if (!tabsInfo.incomingSharedNodes.isEmpty())
    {
        onbShowIncomingSharesClicked();
    }
}

////////////////////////////////
MoveBackupNodeSelector::MoveBackupNodeSelector(QWidget* parent):
    NodeSelector(SelectTypeSPtr(new MoveBackupType), parent)
{
    ui->fBackups->hide();
    ui->fIncomingShares->hide();
}

void MoveBackupNodeSelector::createSpecialisedWidgets()
{
    addCloudDrive();
    mCloudDriveWidget->setShowEmptyView(false);
}

void MoveBackupNodeSelector::onOkButtonClicked()
{
    accept();
}
