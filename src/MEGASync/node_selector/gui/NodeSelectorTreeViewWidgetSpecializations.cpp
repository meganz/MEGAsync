#include "NodeSelectorTreeViewWidgetSpecializations.h"

#include "MegaNodeNames.h"
#include "NodeSelectorModel.h"
#include "NodeSelectorModelSpecialised.h"
#include "NodeSelectorProxyModel.h"
#include "RequestListenerManager.h"
#include "RestoreNodeManager.h"
#include "ui_NodeSelectorTreeViewWidget.h"
#include <MegaApplication.h>

///////////////////////////////////////////////////////////////////
NodeSelectorTreeViewWidgetCloudDrive::NodeSelectorTreeViewWidgetCloudDrive(SelectTypeSPtr mode,
                                                                           QWidget* parent):
    NodeSelectorTreeViewWidget(mode, parent)
{
    setTitle(MegaNodeNames::getCloudDriveName());
}

void NodeSelectorTreeViewWidgetCloudDrive::setShowEmptyView(bool newShowEmptyView)
{
    mShowEmptyView = newShowEmptyView;
}

bool NodeSelectorTreeViewWidgetCloudDrive::isNodeCompatibleWithModel(mega::MegaNode* node)
{
    return MegaSyncApp->getMegaApi()->isInCloud(node);
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
    auto rootIndex = mModel->index(0, 0);
    if (mModel->rowCount(rootIndex) == 0 && showEmptyView())
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
    return Utilities::getIcon(QLatin1String("cloud"),
                              Utilities::AttributeType::SMALL | Utilities::AttributeType::THIN |
                                  Utilities::AttributeType::OUTLINE);
}

NodeSelectorTreeViewWidget::EmptyLabelInfo NodeSelectorTreeViewWidgetCloudDrive::getEmptyLabel()
{
    EmptyLabelInfo info;
    info.description = tr("Cloud drive is empty");
    return info;
}

bool NodeSelectorTreeViewWidgetCloudDrive::isCurrentRootIndexReadOnly()
{
    return false;
}

MegaHandle
    NodeSelectorTreeViewWidgetCloudDrive::findMergedSibling(std::shared_ptr<mega::MegaNode> node)
{
    std::unique_ptr<mega::MegaNode> parentNode(
        MegaSyncApp->getMegaApi()->getParentNode(node.get()));
    if (parentNode)
    {
        std::unique_ptr<mega::MegaNode> foundNode(
            MegaSyncApp->getMegaApi()->getChildNode(parentNode.get(), node->getName()));
        if (foundNode)
        {
            return foundNode->getHandle();
        }
    }

    return mega::INVALID_HANDLE;
}

void NodeSelectorTreeViewWidgetCloudDrive::onRootIndexChanged(const QModelIndex& source_idx)
{
    Q_UNUSED(source_idx)
    ui->tMegaFolders->header()->hideSection(NodeSelectorModel::COLUMN::USER);
    ui->tMegaFolders->header()->hideSection(NodeSelectorModel::COLUMN::ACCESS);

    NodeSelectorTreeViewWidget::onRootIndexChanged(source_idx);
}

/////////////////////////////////////////////////////////////////
NodeSelectorTreeViewWidgetIncomingShares::NodeSelectorTreeViewWidgetIncomingShares(
    SelectTypeSPtr mode,
    QWidget* parent):
    NodeSelectorTreeViewWidget(mode, parent)
{
    setTitle(MegaNodeNames::getIncomingSharesName());
}

bool NodeSelectorTreeViewWidgetIncomingShares::isNodeCompatibleWithModel(mega::MegaNode* node)
{
    if (node->isInShare())
    {
        return true;
    }

    auto access(Utilities::getNodeAccess(node));

    return access != mega::MegaShare::ACCESS_OWNER && access != mega::MegaShare::ACCESS_UNKNOWN;
}

QString NodeSelectorTreeViewWidgetIncomingShares::getRootText()
{
    return MegaNodeNames::getIncomingSharesName();
}

std::unique_ptr<NodeSelectorModel> NodeSelectorTreeViewWidgetIncomingShares::createModel()
{
    return std::unique_ptr<NodeSelectorModelIncomingShares>(new NodeSelectorModelIncomingShares);
}

void NodeSelectorTreeViewWidgetIncomingShares::onRootIndexChanged(const QModelIndex& idx)
{
    if (idx.isValid())
    {
        ui->tMegaFolders->header()->hideSection(NodeSelectorModel::COLUMN::USER);
        ui->tMegaFolders->header()->hideSection(NodeSelectorModel::COLUMN::ACCESS);
    }
    else
    {
        ui->tMegaFolders->header()->showSection(NodeSelectorModel::COLUMN::USER);
        ui->tMegaFolders->header()->showSection(NodeSelectorModel::COLUMN::ACCESS);
    }

    ui->tMegaFolders->header()->hideSection(NodeSelectorModel::COLUMN::ADDED_DATE);

    NodeSelectorTreeViewWidget::onRootIndexChanged(idx);

    // Fill Incoming info
    QModelIndex in_share_idx = getParentIncomingShareByIndex(idx);
    auto item(NodeSelectorModel::getItemByIndex(in_share_idx));
    if (in_share_idx.isValid() && item)
    {
        in_share_idx = in_share_idx.sibling(in_share_idx.row(), NodeSelectorModel::COLUMN::USER);
        QPixmap folderPixmap = qvariant_cast<QPixmap>(idx.data(Qt::DecorationRole));
        QPixmap pm = qvariant_cast<QPixmap>(in_share_idx.data(Qt::DecorationRole));
        ui->sh_folderIcon->setIcon(folderPixmap);
        ui->sh_userIcon->setIcon(pm);

        in_share_idx = in_share_idx.sibling(in_share_idx.row(), NodeSelectorModel::COLUMN::ACCESS);
        QPixmap accessPixmap = qvariant_cast<QPixmap>(in_share_idx.data(Qt::DecorationRole));
        ui->sh_accessIcon->setIcon(accessPixmap);
        ui->sh_accessLabel->setText(in_share_idx.data(Qt::DisplayRole).toString());

        ui->sh_folderName->setText(idx.data(Qt::DisplayRole).toString());
        ui->sh_userEmail->setText(item->getOwnerEmail());
        ui->sh_userName->setText(item->getOwnerName());
        ui->incomingInfo->setVisible(true);
        ui->lFolderName->setVisible(false);
    }
    else
    {
        ui->incomingInfo->setVisible(false);
    }
}

bool NodeSelectorTreeViewWidgetIncomingShares::isCurrentRootIndexReadOnly()
{
    auto rootIndex(ui->tMegaFolders->rootIndex());
    if (rootIndex.isValid())
    {
        auto rootNode = mProxyModel->getNode(rootIndex);
        if (rootNode)
        {
            return MegaSyncApp->getMegaApi()->getAccess(rootNode.get()) <=
                   mega::MegaShare::ACCESS_READ;
        }
    }

    return true;
}

bool NodeSelectorTreeViewWidgetIncomingShares::isSelectionReadOnly(const QModelIndexList& selection)
{
    bool anyReadOnly(false);

    foreach(auto index, selection)
    {
        auto rootIndex(getRootIndexFromIndex(index));
        if (rootIndex.isValid())
        {
            auto rootNode = mProxyModel->getNode(rootIndex);
            if (rootNode)
            {
                if (MegaSyncApp->getMegaApi()->getAccess(rootNode.get()) <=
                    mega::MegaShare::ACCESS_READ)
                {
                    anyReadOnly = true;
                    break;
                }
            }
        }
    }

    return anyReadOnly;
}

bool NodeSelectorTreeViewWidgetIncomingShares::isCurrentSelectionReadOnly()
{
    return isSelectionReadOnly(ui->tMegaFolders->selectedRows());
}

QIcon NodeSelectorTreeViewWidgetIncomingShares::getEmptyIcon()
{
    return Utilities::getIcon(QLatin1String("folder-users"),
                              Utilities::AttributeType::SMALL | Utilities::AttributeType::THIN |
                                  Utilities::AttributeType::OUTLINE);
}

NodeSelectorTreeViewWidget::EmptyLabelInfo NodeSelectorTreeViewWidgetIncomingShares::getEmptyLabel()
{
    EmptyLabelInfo info;
    if (std::dynamic_pointer_cast<SyncType>(mSelectType))
    {
        info.title = tr("No incoming shares you can sync");
        info.description = tr("You can only sync a shared folder if you’ve been given full access");
    }
    else
    {
        info.description = tr("No incoming shares");
    }

    return info;
}

/////////////////////////////////////////////////////////////////
NodeSelectorTreeViewWidgetBackups::NodeSelectorTreeViewWidgetBackups(SelectTypeSPtr mode,
                                                                     QWidget* parent):
    NodeSelectorTreeViewWidget(mode, parent)
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
    return Utilities::getIcon(QLatin1String("devices"),
                              Utilities::AttributeType::SMALL | Utilities::AttributeType::THIN |
                                  Utilities::AttributeType::OUTLINE);
}

void NodeSelectorTreeViewWidgetBackups::onRootIndexChanged(const QModelIndex& idx)
{
    Q_UNUSED(idx)
    ui->tMegaFolders->header()->hideSection(NodeSelectorModel::COLUMN::USER);
    ui->tMegaFolders->header()->hideSection(NodeSelectorModel::COLUMN::ACCESS);

    NodeSelectorTreeViewWidget::onRootIndexChanged(idx);
}

/////////////////////////////////////////////////////////////////

NodeSelectorTreeViewWidgetSearch::NodeSelectorTreeViewWidgetSearch(SelectTypeSPtr mode,
                                                                   QWidget* parent):
    NodeSelectorTreeViewWidget(mode, parent),
    mHasRows(false)

{
    connect(ui->cloudDriveSearch,
            &TabSelector::clicked,
            this,
            &NodeSelectorTreeViewWidgetSearch::onCloudDriveSearchClicked);
    connect(ui->incomingSharesSearch,
            &TabSelector::clicked,
            this,
            &NodeSelectorTreeViewWidgetSearch::onIncomingSharesSearchClicked);
    connect(ui->backupsSearch,
            &TabSelector::clicked,
            this,
            &NodeSelectorTreeViewWidgetSearch::onBackupsSearchClicked);
    connect(ui->rubbishSearch,
            &TabSelector::clicked,
            this,
            &NodeSelectorTreeViewWidgetSearch::onRubbishSearchClicked);

    ui->tMegaFolders->loadingView().setDelayTimeToShowInMs(0);
}

void NodeSelectorTreeViewWidgetSearch::search(const QString& text)
{
    mSearchStr = text;

    changeButtonsWidgetSizePolicy(true);
    ui->stackedWidget->setCurrentWidget(ui->treeViewPage);
    setStyleSheet(styleSheet());

    auto search_model = static_cast<NodeSelectorModelSearch*>(mModel.get());
    search_model->searchByText(text);
}

void NodeSelectorTreeViewWidgetSearch::stopSearch()
{
    auto search_model = static_cast<NodeSelectorModelSearch*>(mModel.get());
    search_model->stopSearch();
    mHasRows = false;
}

std::unique_ptr<NodeSelectorProxyModel> NodeSelectorTreeViewWidgetSearch::createProxyModel()
{
    auto proxy = std::unique_ptr<NodeSelectorProxyModelSearch>(new NodeSelectorProxyModelSearch);
    // The search view is the only one with a real proxy model (in terms on filterAcceptsRow)
    connect(proxy.get(),
            &QAbstractItemModel::rowsInserted,
            this,
            &NodeSelectorTreeViewWidget::checkViewOnModelChange);
    connect(proxy.get(),
            &NodeSelectorProxyModelSearch::modeEmpty,
            this,
            &NodeSelectorTreeViewWidget::checkViewOnModelChange);

    return proxy;
}

bool NodeSelectorTreeViewWidgetSearch::isCurrentRootIndexReadOnly()
{
    return true;
}

void NodeSelectorTreeViewWidgetSearch::resetMovingNumber()
{
    mModel->moveProcessedByNumber(mModel->getMoveRequestsCounter());
}

void NodeSelectorTreeViewWidgetSearch::checkSearchButtonsVisibility()
{
    NodeSelectorModelItemSearch::Types searchedTypes = NodeSelectorModelItemSearch::Type::NONE;
    auto searchModel = dynamic_cast<NodeSelectorModelSearch*>(mModel.get());
    if (searchModel)
    {
        searchedTypes = searchModel->searchedTypes();
    }

    ui->backupsSearch->setVisible(
        searchedTypes.testFlag(NodeSelectorModelItemSearch::Type::BACKUP));
    ui->incomingSharesSearch->setVisible(
        searchedTypes.testFlag(NodeSelectorModelItemSearch::Type::INCOMING_SHARE));
    ui->cloudDriveSearch->setVisible(
        searchedTypes.testFlag(NodeSelectorModelItemSearch::Type::CLOUD_DRIVE));
    ui->rubbishSearch->setVisible(
        searchedTypes.testFlag(NodeSelectorModelItemSearch::Type::RUBBISH));
    ui->searchButtonsWidget->setVisible(true);
}

bool NodeSelectorTreeViewWidgetSearch::isNodeCompatibleWithModel(mega::MegaNode* node)
{
    auto nodeName(QString::fromUtf8(node->getName()));
    auto containsText = nodeName.contains(mSearchStr, Qt::CaseInsensitive);
    return containsText;
}

QModelIndex NodeSelectorTreeViewWidgetSearch::getAddedNodeParent(mega::MegaHandle parentHandle)
{
    Q_UNUSED(parentHandle)
    return QModelIndex();
}

void NodeSelectorTreeViewWidgetSearch::makeCustomConnections()
{
    connect(ui->tMegaFolders,
            &NodeSelectorTreeView::restoreClicked,
            mRestoreManager.get(),
            &RestoreNodeManager::onRestoreClicked);
}

NodeSelectorTreeViewWidget::NodeState
    NodeSelectorTreeViewWidgetSearch::getNodeOnModelState(const QModelIndex& index,
                                                          mega::MegaNode* node)
{
    NodeState result(NodeState::DOESNT_EXIST);

    if (mHasRows && node)
    {
        if (index.isValid())
        {
            result = NodeState::EXISTS;
        }
        else if (isNodeCompatibleWithModel(node))
        {
            result = NodeState::ADD;
        }
    }

    return result;
}

void NodeSelectorTreeViewWidgetSearch::onBackupsSearchClicked()
{
    auto proxy_model = static_cast<NodeSelectorProxyModelSearch*>(mProxyModel.get());
    proxy_model->setMode(NodeSelectorModelItemSearch::Type::BACKUP);
    ui->tMegaFolders->setColumnHidden(NodeSelectorModel::USER, true);
    ui->tMegaFolders->setColumnHidden(NodeSelectorModel::ACCESS, true);
    ui->tMegaFolders->setColumnHidden(NodeSelectorModel::ADDED_DATE, false);

    onRootIndexChanged(QModelIndex());
}

void NodeSelectorTreeViewWidgetSearch::onRubbishSearchClicked()
{
    auto proxy_model = static_cast<NodeSelectorProxyModelSearch*>(mProxyModel.get());
    proxy_model->setMode(NodeSelectorModelItemSearch::Type::RUBBISH);
    ui->tMegaFolders->setColumnHidden(NodeSelectorModel::USER, true);
    ui->tMegaFolders->setColumnHidden(NodeSelectorModel::ACCESS, true);
    ui->tMegaFolders->setColumnHidden(NodeSelectorModel::ADDED_DATE, true);

    onRootIndexChanged(QModelIndex());
}

void NodeSelectorTreeViewWidgetSearch::onIncomingSharesSearchClicked()
{
    auto proxy_model = static_cast<NodeSelectorProxyModelSearch*>(mProxyModel.get());
    proxy_model->setMode(NodeSelectorModelItemSearch::Type::INCOMING_SHARE);
    ui->tMegaFolders->setColumnHidden(NodeSelectorModel::USER, false);
    ui->tMegaFolders->setColumnHidden(NodeSelectorModel::ACCESS, false);
    ui->tMegaFolders->setColumnHidden(NodeSelectorModel::ADDED_DATE, true);

    onRootIndexChanged(QModelIndex());
}

void NodeSelectorTreeViewWidgetSearch::onCloudDriveSearchClicked()
{
    auto proxy_model = static_cast<NodeSelectorProxyModelSearch*>(mProxyModel.get());
    proxy_model->setMode(NodeSelectorModelItemSearch::Type::CLOUD_DRIVE);
    ui->tMegaFolders->setColumnHidden(NodeSelectorModel::USER, true);
    ui->tMegaFolders->setColumnHidden(NodeSelectorModel::ACCESS, true);
    ui->tMegaFolders->setColumnHidden(NodeSelectorModel::ADDED_DATE, false);

    onRootIndexChanged(QModelIndex());
}

void NodeSelectorTreeViewWidgetSearch::onItemDoubleClick(const QModelIndex& index)
{
    auto node = qvariant_cast<std::shared_ptr<MegaNode>>(
        index.data(toInt(NodeSelectorModelRoles::NODE_ROLE)));
    emit nodeDoubleClicked(node, true);
}

void NodeSelectorTreeViewWidgetSearch::changeButtonsWidgetSizePolicy(bool state)
{
    auto buttonWidgetSizePolicy = ui->searchButtonsWidget->sizePolicy();
    buttonWidgetSizePolicy.setRetainSizeWhenHidden(state);
    ui->searchButtonsWidget->setSizePolicy(buttonWidgetSizePolicy);
}

QString NodeSelectorTreeViewWidgetSearch::getRootText()
{
    auto resultNumber(mModel->rowCount());
    QString resultString(tr("%n result found", "", resultNumber));
    return resultString;
}

std::unique_ptr<NodeSelectorModel> NodeSelectorTreeViewWidgetSearch::createModel()
{
    auto model = std::unique_ptr<NodeSelectorModelSearch>(
        new NodeSelectorModelSearch(getSelectType()->allowedTypes()));

    connect(model.get(),
            &NodeSelectorModelSearch::nodeTypeHasChanged,
            this,
            &NodeSelectorTreeViewWidgetSearch::modelLoaded);

    mRestoreManager = std::make_shared<RestoreNodeManager>(model.get(), this);

    // Detect if the row count changed
    connect(model.get(),
            &QAbstractItemModel::rowsInserted,
            this,
            &NodeSelectorTreeViewWidgetSearch::updateRootTitle);

    connect(model.get(),
            &QAbstractItemModel::rowsRemoved,
            this,
            &NodeSelectorTreeViewWidgetSearch::updateRootTitle);

    connect(model.get(),
            &QAbstractItemModel::modelReset,
            this,
            &NodeSelectorTreeViewWidgetSearch::updateRootTitle);

    return model;
}

QIcon NodeSelectorTreeViewWidgetSearch::getEmptyIcon()
{
    return Utilities::getIcon(QLatin1String("search"),
                              Utilities::AttributeType::SMALL | Utilities::AttributeType::THIN |
                                  Utilities::AttributeType::OUTLINE);
}

NodeSelectorTreeViewWidget::EmptyLabelInfo NodeSelectorTreeViewWidgetSearch::getEmptyLabel()
{
    EmptyLabelInfo info;
    info.description = tr("No search results");
    return info;
}

void NodeSelectorTreeViewWidgetSearch::modelLoaded()
{
    if (!mModel)
    {
        return;
    }

    changeButtonsWidgetSizePolicy(false);
    NodeSelectorTreeViewWidget::modelLoaded();

    checkSearchButtonsVisibility();

    TabSelector* tabToCheck(nullptr);

    auto tabs = ui->searchButtonsWidget->findChildren<TabSelector*>();
    foreach(auto& tab, tabs)
    {
        if (tab->isVisible() && tab->isSelected())
        {
            tabToCheck = tab;
            break;
        }
        else if (!tabToCheck && tab->isVisible())
        {
            tabToCheck = tab;
        }
    }

    checkAndClick(tabToCheck);

    if (ui->tMegaFolders->model())
    {
        mHasRows = ui->tMegaFolders->model()->rowCount() > 0;
        if (!mHasRows && showEmptyView())
        {
            ui->stackedWidget->setCurrentWidget(ui->emptyPage);
            return;
        }
    }
    ui->stackedWidget->setCurrentWidget(ui->treeViewPage);
}

void NodeSelectorTreeViewWidgetSearch::checkAndClick(TabSelector* tab)
{
    if (tab && tab->isVisible())
    {
        tab->setSelected(true);
    }
}

///////////////////////
NodeSelectorTreeViewWidgetRubbish::NodeSelectorTreeViewWidgetRubbish(SelectTypeSPtr mode,
                                                                     QWidget* parent):
    NodeSelectorTreeViewWidget(mode, parent)
{
    setTitle(MegaNodeNames::getRubbishName());
}

void NodeSelectorTreeViewWidgetRubbish::setShowEmptyView(bool newShowEmptyView)
{
    mShowEmptyView = newShowEmptyView;
}

bool NodeSelectorTreeViewWidgetRubbish::isEmpty() const
{
    auto rootIndex = mModel->index(0, 0);
    return mModel->rowCount(rootIndex) == 0;
}

bool NodeSelectorTreeViewWidgetRubbish::isNodeCompatibleWithModel(mega::MegaNode* node)
{
    return MegaSyncApp->getMegaApi()->isInRubbish(node);
}

void NodeSelectorTreeViewWidgetRubbish::makeCustomConnections()
{
    connect(ui->tMegaFolders,
            &NodeSelectorTreeView::restoreClicked,
            mRestoreManager.get(),
            &RestoreNodeManager::onRestoreClicked);
}

QString NodeSelectorTreeViewWidgetRubbish::getRootText()
{
    return MegaNodeNames::getRubbishName();
}

std::unique_ptr<NodeSelectorModel> NodeSelectorTreeViewWidgetRubbish::createModel()
{
    auto model = std::unique_ptr<NodeSelectorModelRubbish>(new NodeSelectorModelRubbish);

    mRestoreManager = std::make_shared<RestoreNodeManager>(model.get(), this);

    return model;
}

void NodeSelectorTreeViewWidgetRubbish::modelLoaded()
{
    auto rootIndex = mModel->index(0, 0);
    if (mModel->rowCount(rootIndex) == 0 && showEmptyView())
    {
        ui->stackedWidget->setCurrentWidget(ui->emptyPage);

        // The rubbish has been emptied, so we can unset the loading view
        ui->tMegaFolders->loadingView().toggleLoadingScene(false);
    }
    else
    {
        ui->stackedWidget->setCurrentWidget(ui->treeViewPage);
    }
}

QIcon NodeSelectorTreeViewWidgetRubbish::getEmptyIcon()
{
    return Utilities::getIcon(QLatin1String("trash"),
                              Utilities::AttributeType::SMALL | Utilities::AttributeType::THIN |
                                  Utilities::AttributeType::OUTLINE);
}

NodeSelectorTreeViewWidget::EmptyLabelInfo NodeSelectorTreeViewWidgetRubbish::getEmptyLabel()
{
    EmptyLabelInfo info;
    info.description = tr("The Rubbish bin is empty");
    return info;
}

void NodeSelectorTreeViewWidgetRubbish::onRootIndexChanged(const QModelIndex& source_idx)
{
    Q_UNUSED(source_idx)
    ui->tMegaFolders->header()->hideSection(NodeSelectorModel::COLUMN::USER);
    ui->tMegaFolders->header()->hideSection(NodeSelectorModel::COLUMN::ACCESS);

    NodeSelectorTreeViewWidget::onRootIndexChanged(source_idx);
}
