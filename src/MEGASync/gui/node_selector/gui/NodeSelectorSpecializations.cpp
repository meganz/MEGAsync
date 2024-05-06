#include "NodeSelectorSpecializations.h"
#include "megaapi.h"
#include "MegaApplication.h"
#include "QMegaMessageBox.h"
#include "node_selector/gui/NodeSelectorTreeViewWidgetSpecializations.h"
#include "ui_NodeSelector.h"

UploadNodeSelector::UploadNodeSelector(QWidget *parent) : NodeSelector(parent)
{
    ui->fBackups->hide();
    SelectTypeSPtr selectType = SelectTypeSPtr(new UploadType);
    mCloudDriveWidget = new NodeSelectorTreeViewWidgetCloudDrive(selectType);
    mCloudDriveWidget->setObjectName(QString::fromUtf8("CloudDrive"));
    mCloudDriveWidget->setShowEmptyView(false);
    ui->stackedWidget->addWidget(mCloudDriveWidget);
    mIncomingSharesWidget = new NodeSelectorTreeViewWidgetIncomingShares(selectType);
    mIncomingSharesWidget->setObjectName(QString::fromUtf8("IncomingShares"));
    ui->stackedWidget->addWidget(mIncomingSharesWidget);
    makeConnections(selectType);
}

void UploadNodeSelector::checkSelection()
{
    auto node = getSelectedNode();
    if(node)
    {
        int access = getNodeAccess(node);
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

DownloadNodeSelector::DownloadNodeSelector(QWidget *parent) : NodeSelector(parent)
{
    setWindowTitle(tr("Download"));
    SelectTypeSPtr selectType = SelectTypeSPtr(new DownloadType);
    mCloudDriveWidget = new NodeSelectorTreeViewWidgetCloudDrive(selectType);
    mCloudDriveWidget->setObjectName(QString::fromUtf8("CloudDrive"));
    ui->stackedWidget->addWidget(mCloudDriveWidget);
    mIncomingSharesWidget = new NodeSelectorTreeViewWidgetIncomingShares(selectType);
    mIncomingSharesWidget->setObjectName(QString::fromUtf8("IncomingShares"));
    ui->stackedWidget->addWidget(mIncomingSharesWidget);
    mBackupsWidget = new NodeSelectorTreeViewWidgetBackups(selectType);
    mBackupsWidget->setObjectName(QString::fromUtf8("Backups"));
    ui->stackedWidget->addWidget(mBackupsWidget);
    makeConnections(selectType);
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

SyncNodeSelector::SyncNodeSelector(QWidget *parent) : NodeSelector(parent)
{
    ui->fBackups->hide();
    SelectTypeSPtr selectType = SelectTypeSPtr(new SyncType);
    mCloudDriveWidget = new NodeSelectorTreeViewWidgetCloudDrive(selectType);
    mCloudDriveWidget->setObjectName(QString::fromUtf8("CloudDrive"));
    mCloudDriveWidget->setShowEmptyView(false);
    ui->stackedWidget->addWidget(mCloudDriveWidget);
    mIncomingSharesWidget = new NodeSelectorTreeViewWidgetIncomingShares(selectType);
    mIncomingSharesWidget->setObjectName(QString::fromUtf8("IncomingShares"));
    ui->stackedWidget->addWidget(mIncomingSharesWidget);
    makeConnections(selectType);
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

        int access = getNodeAccess(node);
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

StreamNodeSelector::StreamNodeSelector(QWidget *parent) : NodeSelector(parent)
{
    SelectTypeSPtr selectType = SelectTypeSPtr(new StreamType);
    mCloudDriveWidget = new NodeSelectorTreeViewWidgetCloudDrive(selectType);
    mCloudDriveWidget->setObjectName(QString::fromUtf8("CloudDrive"));
    ui->stackedWidget->addWidget(mCloudDriveWidget);
    mIncomingSharesWidget = new NodeSelectorTreeViewWidgetIncomingShares(selectType);
    mIncomingSharesWidget->setObjectName(QString::fromUtf8("IncomingShares"));
    ui->stackedWidget->addWidget(mIncomingSharesWidget);
    mBackupsWidget = new NodeSelectorTreeViewWidgetBackups(selectType);
    mBackupsWidget->setObjectName(QString::fromUtf8("Backups"));
    ui->stackedWidget->addWidget(mBackupsWidget);
    makeConnections(selectType);
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


MoveBackupNodeSelector::MoveBackupNodeSelector(QWidget *parent) : NodeSelector(parent)
{
    ui->fBackups->hide();
    ui->fIncomingShares->hide();
    SelectTypeSPtr selectType = SelectTypeSPtr(new UploadType);
    mCloudDriveWidget = new NodeSelectorTreeViewWidgetCloudDrive(selectType);
    mCloudDriveWidget->setObjectName(QString::fromUtf8("CloudDrive"));
    mCloudDriveWidget->setShowEmptyView(false);
    ui->stackedWidget->addWidget(mCloudDriveWidget);
    makeConnections(selectType);
}
void MoveBackupNodeSelector::checkSelection()
{
    accept();
}
