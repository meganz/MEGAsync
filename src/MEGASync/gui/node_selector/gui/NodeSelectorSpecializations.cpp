#include "NodeSelectorSpecializations.h"

#include "DialogOpener.h"
#include "DuplicatedNodeDialog.h"
#include "megaapi.h"
#include "MegaNodeNames.h"
#include "NodeSelectorModel.h"
#include "NodeSelectorProxyModel.h"
#include "NodeSelectorTreeViewWidgetSpecializations.h"
#include "QMegaMessageBox.h"
#include "RequestListenerManager.h"
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

void UploadNodeSelector::checkSelection()
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

void DownloadNodeSelector::checkSelection()
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

void SyncNodeSelector::checkSelection()
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

void StreamNodeSelector::checkSelection()
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
    setAcceptDrops(true);

#ifndef Q_OS_MACOS
    Qt::WindowFlags flags = Qt::Window;
    this->setWindowFlags(flags);
#ifdef Q_OS_LINUX
    this->setWindowFlags(this->windowFlags());
#endif
#endif
}

void CloudDriveNodeSelector::createSpecialisedWidgets()
{
    addCloudDrive();
    addIncomingShares();
    addBackups();
    addRubbish();

    enableDragAndDrop(true);
}

void CloudDriveNodeSelector::doCustomConnections(NodeSelectorTreeViewWidget* item)
{
    if (item == mRubbishWidget)
    {
        connect(mRubbishWidget,
                &NodeSelectorTreeViewWidgetRubbish::itemsRestoreRequested,
                this,
                &CloudDriveNodeSelector::onItemsRestoreRequested,
                Qt::UniqueConnection);
    }
}

void CloudDriveNodeSelector::enableDragAndDrop(bool enable)
{
    mCloudDriveWidget->enableDragAndDrop(enable);
    mRubbishWidget->enableDragAndDrop(true);
    mIncomingSharesWidget->enableDragAndDrop(true);
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
    if (type == NodeSelectorModel::ActionType::RESTORE)
    {
        if (!handles.isEmpty())
        {
            mCloudDriveWidget->initMovingNodes(handles.size());
            onbShowCloudDriveClicked();
        }

        performItemsToBeMoved(handles, type, true, false);
    }
    else if (type == NodeSelectorModel::ActionType::DELETE)
    {
        mRubbishWidget->initMovingNodes(handles.size());
        performItemsToBeMoved(handles, type, true, false);
    }
    else if (type == NodeSelectorModel::ActionType::DELETE_PERMANENTLY)
    {
        performItemsToBeMoved(handles, type, true, false);
    }
    else if (type == NodeSelectorModel::ActionType::COPY)
    {
        performItemsToBeMoved(handles, type, false, true);
    }
    else
    {
        NodeSelector::onItemsAboutToBeMoved(handles, type);
    }
}

void CloudDriveNodeSelector::onMergeItemsAboutToBeMoved(mega::MegaHandle handle, int type)
{
    if (type == NodeSelectorModel::ActionType::RESTORE)
    {
        // Check with the handle if we are in CD or Incoming
        if (mCloudDriveWidget->increaseMovingNodes())
        {
            onbShowCloudDriveClicked();
        }
    }
    else
    {
        NodeSelector::onMergeItemsAboutToBeMoved(handle, type);
    }
}

void CloudDriveNodeSelector::onItemsRestoreRequested(const QList<mega::MegaHandle>& handles)
{
    if (mRubbishWidget)
    {
        mRubbishWidget->restoreItems(handles);
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

void MoveBackupNodeSelector::checkSelection()
{
    accept();
}
