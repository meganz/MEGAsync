#include "NodeSelectorTreeViewWidgetSpecializations.h"
#include "ui_NodeSelectorTreeViewWidget.h"
#include "../model/NodeSelectorProxyModel.h"
#include "../model/NodeSelectorModel.h"
#include "../model/NodeSelectorModelSpecialised.h"

#include "MegaNodeNames.h"

///////////////////////////////////////////////////////////////////
NodeSelectorTreeViewWidgetCloudDrive::NodeSelectorTreeViewWidgetCloudDrive(SelectTypeSPtr mode, QWidget *parent)
    : NodeSelectorTreeViewWidget(mode, parent)
{
    setTitle(MegaNodeNames::getCloudDriveName());
}

QString NodeSelectorTreeViewWidgetCloudDrive::getRootText()
{
    return MegaNodeNames::getCloudDriveName();
}

std::unique_ptr<NodeSelectorModel> NodeSelectorTreeViewWidgetCloudDrive::createModel()
{
    return std::unique_ptr<NodeSelectorModelCloudDrive>(new NodeSelectorModelCloudDrive);
}

bool NodeSelectorTreeViewWidgetCloudDrive::isModelEmpty()
{
    auto rootIndex = mModel->index(0,0);
    return mModel->rowCount(rootIndex) == 0;
}

QIcon NodeSelectorTreeViewWidgetCloudDrive::getEmptyIcon()
{
    return QIcon(QString::fromUtf8("://images/node_selector/view/cloud.png"));
}

void NodeSelectorTreeViewWidgetCloudDrive::onRootIndexChanged(const QModelIndex &source_idx)
{
    Q_UNUSED(source_idx)
    ui->tMegaFolders->header()->hideSection(NodeSelectorModel::COLUMN::USER);
}

/////////////////////////////////////////////////////////////////
NodeSelectorTreeViewWidgetIncomingShares::NodeSelectorTreeViewWidgetIncomingShares(SelectTypeSPtr mode, QWidget *parent)
    : NodeSelectorTreeViewWidget(mode, parent)
{
    setTitle(MegaNodeNames::getIncomingSharesName());
}

QString NodeSelectorTreeViewWidgetIncomingShares::getRootText()
{
    return MegaNodeNames::getIncomingSharesName();
}

std::unique_ptr<NodeSelectorModel> NodeSelectorTreeViewWidgetIncomingShares::createModel()
{
    return std::unique_ptr<NodeSelectorModelIncomingShares>(new NodeSelectorModelIncomingShares);
}

void NodeSelectorTreeViewWidgetIncomingShares::onRootIndexChanged(const QModelIndex &source_idx)
{
    if(source_idx.isValid())
    {
        QModelIndex in_share_idx = getParentIncomingShareByIndex(source_idx);
        in_share_idx = in_share_idx.sibling(in_share_idx.row(), NodeSelectorModel::COLUMN::USER);
        QPixmap pm = qvariant_cast<QPixmap>(in_share_idx.data(Qt::DecorationRole));
        QString tooltip = in_share_idx.data(Qt::ToolTipRole).toString();
        ui->lOwnerIcon->setToolTip(tooltip);
        ui->lOwnerIcon->setPixmap(pm);
        ui->avatarSpacer->spacerItem()->changeSize(10, 0);
        ui->tMegaFolders->header()->hideSection(NodeSelectorModel::COLUMN::USER);
    }
    else
    {
        ui->tMegaFolders->header()->showSection(NodeSelectorModel::COLUMN::USER);
    }
}

QIcon NodeSelectorTreeViewWidgetIncomingShares::getEmptyIcon()
{
    return QIcon(QString::fromUtf8("://images/node_selector/view/folder_share.png"));
}

/////////////////////////////////////////////////////////////////
NodeSelectorTreeViewWidgetBackups::NodeSelectorTreeViewWidgetBackups(SelectTypeSPtr mode, QWidget *parent)
    : NodeSelectorTreeViewWidget(mode, parent)
{
    setTitle(MegaNodeNames::getBackupsName());
}

QString NodeSelectorTreeViewWidgetBackups::getRootText()
{
    return MegaNodeNames::getBackupsName();
}

std::unique_ptr<NodeSelectorModel> NodeSelectorTreeViewWidgetBackups::createModel()
{
    return std::unique_ptr<NodeSelectorModelBackups>(new NodeSelectorModelBackups);
}

QIcon NodeSelectorTreeViewWidgetBackups::getEmptyIcon()
{
    return QIcon(QString::fromUtf8("://images/node_selector/view/database.png"));
}

void NodeSelectorTreeViewWidgetBackups::onRootIndexChanged(const QModelIndex &source_idx)
{
    Q_UNUSED(source_idx)
    ui->tMegaFolders->header()->hideSection(NodeSelectorModel::COLUMN::USER);
}
/////////////////////////////////////////////////////////////////

NodeSelectorTreeViewWidgetSearch::NodeSelectorTreeViewWidgetSearch(SelectTypeSPtr mode, QWidget *parent)
    : NodeSelectorTreeViewWidget(mode, parent)

{
    ui->cloudDriveSearch->setChecked(true);
    ui->searchButtonsWidget->setVisible(true);
    ui->lFolderName->setText(tr("Searching:"));
    ui->bBack->hide();
    ui->bForward->hide();
    connect(ui->cloudDriveSearch, &QToolButton::clicked, this, &NodeSelectorTreeViewWidgetSearch::onCloudDriveSearchClicked);
    connect(ui->incomingSharesSearch, &QToolButton::clicked, this, &NodeSelectorTreeViewWidgetSearch::onIncomingSharesSearchClicked);
    connect(ui->backupsSearch, &QToolButton::clicked, this, &NodeSelectorTreeViewWidgetSearch::onBackupsSearchClicked);
}

void NodeSelectorTreeViewWidgetSearch::search(const QString &text)
{
    auto search_model = static_cast<NodeSelectorModelSearch*>(mModel.get());
    search_model->searchByText(text);
    ui->searchingText->setText(text);
    ui->searchNotFoundText->setText(text);
    ui->searchingText->setVisible(true);
}

void NodeSelectorTreeViewWidgetSearch::stopSearch()
{
    auto search_model = static_cast<NodeSelectorModelSearch*>(mModel.get());
    search_model->stopSearch();
}

std::unique_ptr<NodeSelectorProxyModel> NodeSelectorTreeViewWidgetSearch::createProxyModel()
{
    return std::unique_ptr<NodeSelectorProxyModelSearch>(new NodeSelectorProxyModelSearch);
}

void NodeSelectorTreeViewWidgetSearch::onBackupsSearchClicked()
{
    auto proxy_model = static_cast<NodeSelectorProxyModelSearch*>(mProxyModel.get());
    proxy_model->setMode(NodeSelectorModelItemSearch::Type::BACKUP);
}

void NodeSelectorTreeViewWidgetSearch::onIncomingSharesSearchClicked()
{
    auto proxy_model = static_cast<NodeSelectorProxyModelSearch*>(mProxyModel.get());
    proxy_model->setMode(NodeSelectorModelItemSearch::Type::INCOMING_SHARE);
}

void NodeSelectorTreeViewWidgetSearch::onCloudDriveSearchClicked()
{
    auto proxy_model = static_cast<NodeSelectorProxyModelSearch*>(mProxyModel.get());
    proxy_model->setMode(NodeSelectorModelItemSearch::Type::CLOUD_DRIVE);
}

void NodeSelectorTreeViewWidgetSearch::onItemDoubleClick(const QModelIndex &index)
{
    auto sourceIndex = mProxyModel->mapToSource(index);
    NodeSelectorModelItem *item = static_cast<NodeSelectorModelItem*>(sourceIndex.internalPointer());
    emit nodeDoubleClicked(item->getNode());
}

QString NodeSelectorTreeViewWidgetSearch::getRootText()
{
    return tr("Searching:");
}

std::unique_ptr<NodeSelectorModel> NodeSelectorTreeViewWidgetSearch::createModel()
{
    return std::unique_ptr<NodeSelectorModelSearch>(new NodeSelectorModelSearch);
}

QIcon NodeSelectorTreeViewWidgetSearch::getEmptyIcon()
{
    return QIcon(QString::fromUtf8("://images/node_selector/view/search.png"));
}

bool NodeSelectorTreeViewWidgetSearch::isModelEmpty()
{
    if(qobject_cast<NodeSelectorModelSearch*>(mModel.get())->isSearching())
    {
        return false;
    }
    return mProxyModel->rowCount() == 0;
}
