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
    ui->stackedWidget->addWidget(mCloudDriveWidget);
    mIncomingSharesWidget = new NodeSelectorTreeViewWidgetIncomingShares(selectType);
    mIncomingSharesWidget->setObjectName(QString::fromUtf8("IncomingShares"));
    ui->stackedWidget->addWidget(mIncomingSharesWidget);
    makeConnections(selectType);
}

bool UploadNodeSelector::isSelectionCorrect()
{
    bool ret = false;
    auto node = getSelectedNode();
    if(node)
    {
        ret = true;
        int access = getNodeAccess(node);
        if (access < mega::MegaShare::ACCESS_READWRITE)
        {
            QMegaMessageBox::warning(nullptr, tr("Error"), tr("You need Read & Write or Full access rights to be able to upload to the selected folder."), QMessageBox::Ok);
            ret = false;
        }
    }
    return ret;
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

bool DownloadNodeSelector::isSelectionCorrect()
{
    bool ret(true);
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

    if(wrongNodes == nodes.size())
    {
        ret = false;
        if(ui->stackedWidget->currentIndex() == CLOUD_DRIVE)
        {
            QMegaMessageBox::warning(nullptr, tr("Error"), tr("The item you selected has been removed. To reselect, close this window and try again.", "", wrongNodes), QMessageBox::Ok);
        }
        else
        {
            QMegaMessageBox::warning(nullptr, tr("Error"), tr("You no longer have access to this item. Ask the owner to share again.", "", wrongNodes), QMessageBox::Ok);
        }
    }
    else if(wrongNodes > 0)
    {
        ret = false;
        QString warningMsg1 = tr("%1 item selected", "", nodes.size()).arg(nodes.size());
        QString warningMsg = tr("%1. %2 has been removed. To reselect, close this window and try again.", "", wrongNodes).arg(warningMsg1).arg(wrongNodes);
        QMegaMessageBox::warning(nullptr, tr("Error"), warningMsg, QMessageBox::Ok);
    }
    return ret;
}

SyncNodeSelector::SyncNodeSelector(QWidget *parent) : NodeSelector(parent)
{
    ui->fBackups->hide();
    SelectTypeSPtr selectType = SelectTypeSPtr(new SyncType);
    mCloudDriveWidget = new NodeSelectorTreeViewWidgetCloudDrive(selectType);
    mCloudDriveWidget->setObjectName(QString::fromUtf8("CloudDrive"));
    ui->stackedWidget->addWidget(mCloudDriveWidget);
    mIncomingSharesWidget = new NodeSelectorTreeViewWidgetIncomingShares(selectType);
    mIncomingSharesWidget->setObjectName(QString::fromUtf8("IncomingShares"));
    ui->stackedWidget->addWidget(mIncomingSharesWidget);
    makeConnections(selectType);
}

bool SyncNodeSelector::isSelectionCorrect()
{
    auto node = getSelectedNode();
    bool ret = false;
    if(node)
    {
        ret = true;
        int access = getNodeAccess(node);
        if (access < mega::MegaShare::ACCESS_FULL)
        {
            QMegaMessageBox::warning(nullptr, tr("Error"), tr("You need Full access right to be able to sync the selected folder."), QMessageBox::Ok);
            ret = false;
        }
        else
        {
            const char* path = mMegaApi->getNodePath(node.get());
            auto check = std::unique_ptr<mega::MegaNode>(mMegaApi->getNodeByPath(path));
            delete [] path;
            if (!check)
            {
                QMegaMessageBox::warning(nullptr, tr("Warning"), tr("Invalid folder for synchronization.\n"
                                                                    "Please, ensure that you don't use characters like '\\' '/' or ':' in your folder names."),
                                         QMessageBox::Ok);
                ret = false;
            }
        }
    }
    return ret;
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

bool StreamNodeSelector::isSelectionCorrect()
{
    auto node = getSelectedNode();
    bool ret = false;
    if(node)
    {
        ret = true;
        if (node->isFolder())
        {
            QMegaMessageBox::warning(nullptr, tr("Error"), tr("Only files can be used for streaming."), QMessageBox::Ok);
            ret = false;
        }
    }
    return ret;
}

