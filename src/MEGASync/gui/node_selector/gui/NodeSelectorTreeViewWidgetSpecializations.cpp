#include "NodeSelectorTreeViewWidgetSpecializations.h"
#include "ui_NodeSelectorTreeViewWidget.h"

#include "MegaNodeNames.h"
#include "../model/NodeSelectorModel.h"
#include "../model/NodeSelectorModelSpecialised.h"

///////////////////////////////////////////////////////////////////
NodeSelectorTreeViewWidgetCloudDrive::NodeSelectorTreeViewWidgetCloudDrive(SelectTypeSPtr mode, QWidget *parent)
    : NodeSelectorTreeViewWidget(mode, parent)
{
    setTitle(MegaNodeNames::getCloudDriveName());
    ui->searchEmptyInfoWidget->setVisible(false);
}

QString NodeSelectorTreeViewWidgetCloudDrive::getRootText()
{
    return MegaNodeNames::getCloudDriveName();
}

std::unique_ptr<NodeSelectorModel> NodeSelectorTreeViewWidgetCloudDrive::getModel()
{
    return std::unique_ptr<NodeSelectorModelCloudDrive>(new NodeSelectorModelCloudDrive);
}

bool NodeSelectorTreeViewWidgetCloudDrive::isModelEmpty()
{
    auto rootIndex = mModel->index(0,0);
    return mModel->rowCount(rootIndex) == 0;
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

std::unique_ptr<NodeSelectorModel> NodeSelectorTreeViewWidgetIncomingShares::getModel()
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

std::unique_ptr<NodeSelectorModel> NodeSelectorTreeViewWidgetBackups::getModel()
{
    return std::unique_ptr<NodeSelectorModelBackups>(new NodeSelectorModelBackups);
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
    ui->searchButtonsWidget->setVisible(true);
}

void NodeSelectorTreeViewWidgetSearch::search(const QString &text)
{
    auto search_model = static_cast<NodeSelectorModelSearch*>(mModel.get());
    search_model->searchByText(text);
    ui->searchingText->setText(text);
    ui->searchingText->setVisible(true);
    ui->cloudDriveSearch->setChecked(true);
}

QString NodeSelectorTreeViewWidgetSearch::getRootText()
{
    return QString::fromUtf8("Searching:");
}

std::unique_ptr<NodeSelectorModel> NodeSelectorTreeViewWidgetSearch::getModel()
{
    return std::unique_ptr<NodeSelectorModelSearch>(new NodeSelectorModelSearch);
}
