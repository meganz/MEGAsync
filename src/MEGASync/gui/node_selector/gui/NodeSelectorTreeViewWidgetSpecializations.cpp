#include "NodeSelectorTreeViewWidgetSpecializations.h"

#include "MegaNodeNames.h"
#include "NodeSelectorModel.h"
#include "NodeSelectorModelSpecialised.h"
#include "NodeSelectorProxyModel.h"
#include "RequestListenerManager.h"
#include "ui_NodeSelectorTreeViewWidget.h"
#include <MegaApplication.h>

///////////////////////////////////////////////////////////////////

void RestoreNodeManager::onRestoreClicked(const QList<mega::MegaHandle>& handles)
{
    mRestoredItems = handles;

    QList<QPair<mega::MegaHandle, std::shared_ptr<mega::MegaNode>>> moveHandles;

    foreach(auto handle, handles)
    {
        auto node = std::shared_ptr<MegaNode>(MegaSyncApp->getMegaApi()->getNodeByHandle(handle));
        if (node)
        {
            auto newParent = std::shared_ptr<MegaNode>(
                MegaSyncApp->getMegaApi()->getNodeByHandle(node->getRestoreHandle()));
            moveHandles.append(
                qMakePair<mega::MegaHandle, std::shared_ptr<mega::MegaNode>>(handle, newParent));
        }
    }

    mModel->processNodesAndCheckConflicts(moveHandles,
                                          MegaSyncApp->getRubbishNode(),
                                          NodeSelectorModel::ActionType::RESTORE);
}

///////////////////////////////////////////////////////////////////
NodeSelectorTreeViewWidgetCloudDrive::NodeSelectorTreeViewWidgetCloudDrive(SelectTypeSPtr mode, QWidget *parent)
    : NodeSelectorTreeViewWidget(mode, parent)
{
    setTitle(MegaNodeNames::getCloudDriveName());
    ui->searchEmptyInfoWidget->hide();
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
    auto rootIndex = mModel->index(0,0);
    if(mModel->rowCount(rootIndex) == 0 && showEmptyView())
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

void NodeSelectorTreeViewWidgetCloudDrive::onRootIndexChanged(const QModelIndex &source_idx)
{
    Q_UNUSED(source_idx)
    ui->tMegaFolders->header()->hideSection(NodeSelectorModel::COLUMN::USER);
    ui->tMegaFolders->header()->hideSection(NodeSelectorModel::COLUMN::ACCESS);
}

/////////////////////////////////////////////////////////////////
NodeSelectorTreeViewWidgetIncomingShares::NodeSelectorTreeViewWidgetIncomingShares(SelectTypeSPtr mode, QWidget *parent)
    : NodeSelectorTreeViewWidget(mode, parent)
{
    setTitle(MegaNodeNames::getIncomingSharesName());
    ui->searchEmptyInfoWidget->hide();
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

void NodeSelectorTreeViewWidgetIncomingShares::onRootIndexChanged(const QModelIndex &idx)
{
    if(idx.isValid())
    {
        QModelIndex in_share_idx = getParentIncomingShareByIndex(idx);
        in_share_idx = in_share_idx.sibling(in_share_idx.row(), NodeSelectorModel::COLUMN::USER);
        QPixmap pm = qvariant_cast<QPixmap>(in_share_idx.data(Qt::DecorationRole));
        QString tooltip = in_share_idx.data(Qt::ToolTipRole).toString();
        ui->lOwnerIcon->setToolTip(tooltip);
        ui->lOwnerIcon->setPixmap(pm);
        auto item(NodeSelectorModel::getItemByIndex(idx));
        if (item)
        {
            ui->lAccess->show();
            ui->lAccess->setText(Utilities::getNodeStringAccess(item->getNode().get()));
        }
        ui->avatarSpacer->spacerItem()->changeSize(10, 0);
        ui->tMegaFolders->header()->hideSection(NodeSelectorModel::COLUMN::USER);
        ui->tMegaFolders->header()->hideSection(NodeSelectorModel::COLUMN::ACCESS);
    }
    else
    {
        ui->lAccess->hide();
        ui->tMegaFolders->header()->showSection(NodeSelectorModel::COLUMN::USER);
        ui->tMegaFolders->header()->showSection(NodeSelectorModel::COLUMN::ACCESS);
    }
}

bool NodeSelectorTreeViewWidgetIncomingShares::isCurrentRootIndexReadOnly()
{
    auto rootIndex(ui->tMegaFolders->rootIndex());
    if(rootIndex.isValid())
    {
        auto rootNode = mProxyModel->getNode(rootIndex);
        if(rootNode)
        {
            return MegaSyncApp->getMegaApi()->getAccess(rootNode.get()) <= mega::MegaShare::ACCESS_READ;
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
        if(rootIndex.isValid())
        {
            auto rootNode = mProxyModel->getNode(rootIndex);
            if(rootNode)
            {
                if(MegaSyncApp->getMegaApi()->getAccess(rootNode.get()) <= mega::MegaShare::ACCESS_READ)
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
    return QIcon(QString::fromUtf8("://images/node_selector/view/folder_share.png"));
}

/////////////////////////////////////////////////////////////////
NodeSelectorTreeViewWidgetBackups::NodeSelectorTreeViewWidgetBackups(SelectTypeSPtr mode, QWidget *parent)
    : NodeSelectorTreeViewWidget(mode, parent)
{
    setTitle(MegaNodeNames::getBackupsName());
    ui->searchEmptyInfoWidget->hide();
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

void NodeSelectorTreeViewWidgetBackups::onRootIndexChanged(const QModelIndex &idx)
{
    Q_UNUSED(idx)
    ui->tMegaFolders->header()->hideSection(NodeSelectorModel::COLUMN::USER);
    ui->tMegaFolders->header()->hideSection(NodeSelectorModel::COLUMN::ACCESS);
}
/////////////////////////////////////////////////////////////////

NodeSelectorTreeViewWidgetSearch::NodeSelectorTreeViewWidgetSearch(SelectTypeSPtr mode, QWidget *parent)
    : NodeSelectorTreeViewWidget(mode, parent)
    , mHasRows(false)

{
    setTitleText(tr("Searching:"));
    ui->bBack->hide();
    ui->bForward->hide();
    connect(ui->cloudDriveSearch, &QToolButton::clicked, this, &NodeSelectorTreeViewWidgetSearch::onCloudDriveSearchClicked);
    connect(ui->incomingSharesSearch, &QToolButton::clicked, this, &NodeSelectorTreeViewWidgetSearch::onIncomingSharesSearchClicked);
    connect(ui->backupsSearch, &QToolButton::clicked, this, &NodeSelectorTreeViewWidgetSearch::onBackupsSearchClicked);
    connect(ui->rubbishSearch,
            &QToolButton::clicked,
            this,
            &NodeSelectorTreeViewWidgetSearch::onRubbishSearchClicked);
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
    auto containsText = nodeName.contains(ui->searchingText->text(),Qt::CaseInsensitive);
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
    NodeSelectorTreeViewWidgetSearch::getNodeOnModelState(mega::MegaNode* node)
{
    NodeState result(NodeState::DOESNT_EXIST);

    if (mHasRows && node)
    {
        if (mModel->findIndexByNodeHandle(node->getHandle(), QModelIndex()).isValid())
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
}

void NodeSelectorTreeViewWidgetSearch::onRubbishSearchClicked()
{
    auto proxy_model = static_cast<NodeSelectorProxyModelSearch*>(mProxyModel.get());
    proxy_model->setMode(NodeSelectorModelItemSearch::Type::RUBBISH);
    ui->tMegaFolders->setColumnHidden(NodeSelectorModel::USER, true);
    ui->tMegaFolders->setColumnHidden(NodeSelectorModel::ACCESS, true);
}

void NodeSelectorTreeViewWidgetSearch::onIncomingSharesSearchClicked()
{
    auto proxy_model = static_cast<NodeSelectorProxyModelSearch*>(mProxyModel.get());
    proxy_model->setMode(NodeSelectorModelItemSearch::Type::INCOMING_SHARE);
    ui->tMegaFolders->setColumnHidden(NodeSelectorModel::USER, false);
    ui->tMegaFolders->setColumnHidden(NodeSelectorModel::ACCESS, false);
}

void NodeSelectorTreeViewWidgetSearch::onCloudDriveSearchClicked()
{
    auto proxy_model = static_cast<NodeSelectorProxyModelSearch*>(mProxyModel.get());
    proxy_model->setMode(NodeSelectorModelItemSearch::Type::CLOUD_DRIVE);
    ui->tMegaFolders->setColumnHidden(NodeSelectorModel::USER, true);
    ui->tMegaFolders->setColumnHidden(NodeSelectorModel::ACCESS, true);
}

void NodeSelectorTreeViewWidgetSearch::onItemDoubleClick(const QModelIndex &index)
{
    auto node = qvariant_cast<std::shared_ptr<MegaNode>>(index.data(toInt(NodeSelectorModelRoles::NODE_ROLE)));
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
    return tr("Searching:");
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

    return model;
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
    NodeSelectorTreeViewWidget::modelLoaded();

    checkSearchButtonsVisibility();

    QToolButton* buttonToCheck(nullptr);

    auto buttons = ui->searchButtonsWidget->findChildren<QToolButton*>();
    foreach(auto& button, buttons)
    {
        if(button->isVisible() && button->isChecked())
        {
            buttonToCheck = button;
            break;
        }
        else if(!buttonToCheck && button->isVisible())
        {
            buttonToCheck = button;
        }
    }

    checkAndClick(buttonToCheck);

    if(ui->tMegaFolders->model())
    {
        mHasRows = ui->tMegaFolders->model()->rowCount() > 0;
        if(!mHasRows && showEmptyView())
        {
            ui->stackedWidget->setCurrentWidget(ui->emptyPage);
            return;
        }
    }
    ui->stackedWidget->setCurrentWidget(ui->treeViewPage);
}

void NodeSelectorTreeViewWidgetSearch::checkAndClick(QToolButton* button)
{
    if(button  && button->isVisible())
    {
        button->setChecked(true);
        emit button->clicked(true);
    }
}

///////////////////////
NodeSelectorTreeViewWidgetRubbish::NodeSelectorTreeViewWidgetRubbish(SelectTypeSPtr mode, QWidget *parent)
    : NodeSelectorTreeViewWidget(mode, parent)
{
    setTitle(MegaNodeNames::getRubbishName());
    ui->searchEmptyInfoWidget->hide();
}

void NodeSelectorTreeViewWidgetRubbish::setShowEmptyView(bool newShowEmptyView)
{
    mShowEmptyView = newShowEmptyView;
}

bool NodeSelectorTreeViewWidgetRubbish::isEmpty() const
{
    auto rootIndex = mModel->index(0,0);
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
    auto rootIndex = mModel->index(0,0);
    if(mModel->rowCount(rootIndex) == 0 && showEmptyView())
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
    return QIcon(QString::fromUtf8("://images/node_selector/view/rubbish_empty.png"));
}

void NodeSelectorTreeViewWidgetRubbish::onRootIndexChanged(const QModelIndex &source_idx)
{
    Q_UNUSED(source_idx)
    ui->tMegaFolders->header()->hideSection(NodeSelectorModel::COLUMN::USER);
    ui->tMegaFolders->header()->hideSection(NodeSelectorModel::COLUMN::ACCESS);
}
