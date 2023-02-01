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

void NodeSelectorTreeViewWidgetCloudDrive::modelLoaded()
{
    auto rootIndex = mModel->index(0,0);
    if(mModel->rowCount(rootIndex) == 0)
    {
        ui->stackedWidget->setCurrentWidget(ui->emptyPage);
    }
    else
    {
        ui->stackedWidget->setCurrentWidget(ui->treeViewPage);
    }
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
    ui->lFolderName->setText(tr("Searching:"));
    ui->bBack->hide();
    ui->bForward->hide();
    connect(ui->cloudDriveSearch, &QToolButton::clicked, this, &NodeSelectorTreeViewWidgetSearch::onCloudDriveSearchClicked);
    connect(ui->incomingSharesSearch, &QToolButton::clicked, this, &NodeSelectorTreeViewWidgetSearch::onIncomingSharesSearchClicked);
    connect(ui->backupsSearch, &QToolButton::clicked, this, &NodeSelectorTreeViewWidgetSearch::onBackupsSearchClicked);
}

void NodeSelectorTreeViewWidgetSearch::search(const QString &text)
{
    changeButtonsWidgetSizePolicy(true);
    ui->stackedWidget->setCurrentWidget(ui->treeViewPage);
    ui->searchButtonsWidget->setVisible(false);
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
    ui->tMegaFolders->setColumnHidden(NodeSelectorModel::USER, true);
}

void NodeSelectorTreeViewWidgetSearch::onIncomingSharesSearchClicked()
{
    auto proxy_model = static_cast<NodeSelectorProxyModelSearch*>(mProxyModel.get());
    proxy_model->setMode(NodeSelectorModelItemSearch::Type::INCOMING_SHARE);
    ui->tMegaFolders->setColumnHidden(NodeSelectorModel::USER, false);
}

void NodeSelectorTreeViewWidgetSearch::onCloudDriveSearchClicked()
{
    auto proxy_model = static_cast<NodeSelectorProxyModelSearch*>(mProxyModel.get());
    proxy_model->setMode(NodeSelectorModelItemSearch::Type::CLOUD_DRIVE);
    ui->tMegaFolders->setColumnHidden(NodeSelectorModel::USER, true);
}

void NodeSelectorTreeViewWidgetSearch::onItemDoubleClick(const QModelIndex &index)
{
    auto sourceIndex = mProxyModel->mapToSource(index);
    NodeSelectorModelItem *item = static_cast<NodeSelectorModelItem*>(sourceIndex.internalPointer());
    emit nodeDoubleClicked(item->getNode(), true);
}

void NodeSelectorTreeViewWidgetSearch::changeButtonsWidgetSizePolicy(bool state)
{
    auto buttonWidgetSizePolicy = ui->searchButtonsWidget->sizePolicy();
    buttonWidgetSizePolicy.setRetainSizeWhenHidden(state);
    ui->searchButtonsWidget->setSizePolicy(buttonWidgetSizePolicy);
}

bool NodeSelectorTreeViewWidgetSearch::nothingChecked() const
{
   return !ui->backupsSearch->isChecked() && !ui->cloudDriveSearch->isChecked() && !ui->incomingSharesSearch->isChecked();
}

QString NodeSelectorTreeViewWidgetSearch::getRootText()
{
    return tr("Searching:");
}

std::unique_ptr<NodeSelectorModel> NodeSelectorTreeViewWidgetSearch::createModel()
{
    return std::unique_ptr<NodeSelectorModelSearch>(new NodeSelectorModelSearch(getSelectType()->allowedTypes()));
}

QIcon NodeSelectorTreeViewWidgetSearch::getEmptyIcon()
{
    return QIcon(QString::fromUtf8("://images/node_selector/view/search.png"));
}

void NodeSelectorTreeViewWidgetSearch::modelLoaded()
{
    if(!mModel)
    {
        return;
    }

    changeButtonsWidgetSizePolicy(false);

    if(mModel->rowCount() == 0)
    {
        ui->searchButtonsWidget->setVisible(false);
        ui->stackedWidget->setCurrentWidget(ui->emptyPage);
        return;
    }
    else
    {
        ui->stackedWidget->setCurrentWidget(ui->treeViewPage);
    }

    NodeSelectorModelItemSearch::Types searchedTypes = NodeSelectorModelItemSearch::Type::NONE;
    auto searchModel = dynamic_cast<NodeSelectorModelSearch*>(mModel.get());
    if(searchModel)
    {
        searchedTypes = searchModel->searchedTypes();
    }

    ui->backupsSearch->setVisible(searchedTypes.testFlag(NodeSelectorModelItemSearch::Type::BACKUP));
    ui->incomingSharesSearch->setVisible(searchedTypes.testFlag(NodeSelectorModelItemSearch::Type::INCOMING_SHARE));
    ui->cloudDriveSearch->setVisible(searchedTypes.testFlag(NodeSelectorModelItemSearch::Type::CLOUD_DRIVE));
    ui->searchButtonsWidget->setVisible(true);

    if(ui->cloudDriveSearch->isChecked() && !ui->cloudDriveSearch->isVisible())
    {
        if(ui->incomingSharesSearch->isVisible())
        {
            ui->incomingSharesSearch->setChecked(true);
            emit ui->incomingSharesSearch->clicked(true);
        }
        else if(ui->backupsSearch->isVisible())
        {
            ui->backupsSearch->setChecked(true);
            emit ui->backupsSearch->clicked(true);
        }
    }
    else if(ui->incomingSharesSearch->isChecked() && !ui->incomingSharesSearch->isVisible())
    {
        if(ui->cloudDriveSearch->isVisible())
        {
            ui->cloudDriveSearch->setChecked(true);
            emit ui->cloudDriveSearch->clicked(true);
        }
        else if(ui->backupsSearch->isVisible())
        {
            ui->backupsSearch->setChecked(true);
            emit ui->backupsSearch->clicked(true);
        }
    }
    else if(ui->backupsSearch->isChecked() && !ui->backupsSearch->isVisible())
    {
        if(ui->cloudDriveSearch->isVisible())
        {
            ui->cloudDriveSearch->setChecked(true);
            emit ui->cloudDriveSearch->clicked(true);
        }
        else if(ui->incomingSharesSearch->isVisible())
        {
            ui->incomingSharesSearch->setChecked(true);
            emit ui->incomingSharesSearch->clicked(true);
        }
    }
    else if(nothingChecked())
    {
        if(ui->cloudDriveSearch->isVisible())
        {
            ui->cloudDriveSearch->setChecked(true);
            emit ui->cloudDriveSearch->clicked(true);
        }
        else if(ui->incomingSharesSearch->isVisible())
        {
            ui->incomingSharesSearch->setChecked(true);
            emit ui->incomingSharesSearch->clicked(true);
        }
        else if(ui->backupsSearch->isVisible())
        {
            ui->backupsSearch->setChecked(true);
            emit ui->backupsSearch->clicked(true);
        }
    }

}
