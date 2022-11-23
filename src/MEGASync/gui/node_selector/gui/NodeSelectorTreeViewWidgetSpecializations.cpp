#include "NodeSelectorTreeViewWidgetSpecializations.h"
#include "ui_NodeSelectorTreeViewWidget.h"

#include "../model/NodeSelectorModel.h"
#include "../model/NodeSelectorModelSpecialised.h"

///////////////////////////////////////////////////////////////////
NodeSelectorTreeViewWidgetCloudDrive::NodeSelectorTreeViewWidgetCloudDrive(QWidget *parent)
    : NodeSelectorTreeViewWidget(parent)
{
    setTitle(tr(CLD_DRIVE));
}

QString NodeSelectorTreeViewWidgetCloudDrive::getRootText()
{
    return tr(CLD_DRIVE);
}

std::unique_ptr<NodeSelectorModel> NodeSelectorTreeViewWidgetCloudDrive::getModel()
{
    return std::unique_ptr<NodeSelectorModelCloudDrive>(new NodeSelectorModelCloudDrive);
}

void NodeSelectorTreeViewWidgetCloudDrive::setRootIndex_Reimplementation(const QModelIndex &source_idx)
{
    Q_UNUSED(source_idx)
    ui->tMegaFolders->header()->hideSection(NodeSelectorModel::COLUMN::USER);
}

/////////////////////////////////////////////////////////////////
NodeSelectorTreeViewWidgetIncomingShares::NodeSelectorTreeViewWidgetIncomingShares(QWidget *parent)
    : NodeSelectorTreeViewWidget(parent)
{
    setTitle(tr(IN_SHARES));
}

QString NodeSelectorTreeViewWidgetIncomingShares::getRootText()
{
    return tr(IN_SHARES);
}

std::unique_ptr<NodeSelectorModel> NodeSelectorTreeViewWidgetIncomingShares::getModel()
{
    return std::unique_ptr<NodeSelectorModelIncomingShares>(new NodeSelectorModelIncomingShares);
}

void NodeSelectorTreeViewWidgetIncomingShares::setRootIndex_Reimplementation(const QModelIndex &source_idx)
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

/////////////////////////////////////////////////////////////////
NodeSelectorTreeViewWidgetBackups::NodeSelectorTreeViewWidgetBackups(QWidget *parent)
    : NodeSelectorTreeViewWidget(parent)
{
    setTitle(tr(BACKUPS));
}

QString NodeSelectorTreeViewWidgetBackups::getRootText()
{
    return tr(BACKUPS);
}

std::unique_ptr<NodeSelectorModel> NodeSelectorTreeViewWidgetBackups::getModel()
{
    return std::unique_ptr<NodeSelectorModelBackups>(new NodeSelectorModelBackups);
}

void NodeSelectorTreeViewWidgetBackups::setRootIndex_Reimplementation(const QModelIndex &source_idx)
{
    Q_UNUSED(source_idx)
    ui->tMegaFolders->header()->hideSection(NodeSelectorModel::COLUMN::USER);
}
