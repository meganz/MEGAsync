#include "NodeSelectorTreeViewWidgetSpecializations.h"

#include "DeviceNames.h"
#include "MegaNodeNames.h"
#include "NodeSelectorDelegates.h"
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
    NodeSelectorTreeViewWidget(mode, TabItem::CLOUD_DRIVE, parent)
{}

void NodeSelectorTreeViewWidgetCloudDrive::setShowEmptyView(bool newShowEmptyView)
{
    mShowEmptyView = newShowEmptyView;
}

bool NodeSelectorTreeViewWidgetCloudDrive::isNodeCompatibleWithModel(mega::MegaNode* node)
{
    return MegaSyncApp->getMegaApi()->isInCloud(node);
}

QString NodeSelectorTreeViewWidgetCloudDrive::getRootText() const
{
    return MegaNodeNames::getCloudDriveName();
}

std::unique_ptr<NodeSelectorModel> NodeSelectorTreeViewWidgetCloudDrive::createModel()
{
    return std::unique_ptr<NodeSelectorModelCloudDrive>(new NodeSelectorModelCloudDrive);
}

void NodeSelectorTreeViewWidgetCloudDrive::setViewPage()
{
    if (mProxyModel->rowCount(getCurrentRootIndex()) == 0 && showEmptyView())
    {
        setCurrentPage(ViewType::ROOT_EMPTY);
    }
    else
    {
        NodeSelectorTreeViewWidget::setViewPage();
    }
}

SelectType::EmptyPageInfo NodeSelectorTreeViewWidgetCloudDrive::getEmptyRootPageInfo()
{
    return mSelectType->getEmptyCloudDrivePage();
}

bool NodeSelectorTreeViewWidgetCloudDrive::isCurrentRootIndexReadOnly() const
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

/////////////////////////////////////////////////////////////////
/// \brief NodeSelectorTreeViewWidgetIncomingShares::NodeSelectorTreeViewWidgetIncomingShares
/// \param mode
/// \param parent

NodeSelectorTreeViewWidgetIncomingShares::NodeSelectorTreeViewWidgetIncomingShares(
    SelectTypeSPtr mode,
    QWidget* parent):
    NodeSelectorTreeViewWidget(mode, TabItem::SHARES, parent)
{}

bool NodeSelectorTreeViewWidgetIncomingShares::isNodeCompatibleWithModel(mega::MegaNode* node)
{
    if (node->isInShare())
    {
        return true;
    }

    auto access(Utilities::getNodeAccess(node));

    return access != mega::MegaShare::ACCESS_OWNER && access != mega::MegaShare::ACCESS_UNKNOWN;
}

std::optional<IncomingShareHeaderData>
    NodeSelectorTreeViewWidgetIncomingShares::incomingShareHeaderData() const
{
    const auto rootIndex = getCurrentRootIndex();
    if (!rootIndex.isValid())
    {
        return std::nullopt;
    }

    // The header must only be shown when the current root folder is the incoming
    // share itself, not when navigating into one of its nested folders.
    auto rootItem = NodeSelectorModel::getItemByIndex(rootIndex);
    if (!rootItem || !rootItem->getNode() || !rootItem->getNode()->isInShare())
    {
        return std::nullopt;
    }

    IncomingShareHeaderData incomingInfo;
    incomingInfo.folderName = rootIndex.data(Qt::DisplayRole).toString();

    auto inShareIndex = getParentIncomingShareByIndex(rootIndex);
    auto item = NodeSelectorModel::getItemByIndex(inShareIndex);
    if (inShareIndex.isValid() && item)
    {
        inShareIndex = inShareIndex.sibling(inShareIndex.row(), NodeSelectorModel::Column::USER);
        incomingInfo.userIcon = qvariant_cast<QPixmap>(inShareIndex.data(Qt::DecorationRole));

        inShareIndex = inShareIndex.sibling(inShareIndex.row(), NodeSelectorModel::Column::ACCESS);
        incomingInfo.accessIcon = qvariant_cast<QPixmap>(inShareIndex.data(Qt::DecorationRole));
        incomingInfo.accessLabel = inShareIndex.data(Qt::DisplayRole).toString();
        incomingInfo.accessType =
            inShareIndex.data(toInt(NodeSelectorModelRoles::ACCESS_ROLE)).toInt();
        incomingInfo.userEmail = item->getOwnerEmail();
        incomingInfo.userName = item->getOwnerName();
    }

    return incomingInfo;
}

void NodeSelectorTreeViewWidgetIncomingShares::makeViewConnections()
{
    auto incomingSharesModel = qobject_cast<NodeSelectorModelIncomingShares*>(mModel.get());
    if (!incomingSharesModel)
    {
        return;
    }

    connect(
        incomingSharesModel,
        &NodeSelectorModelIncomingShares::incomingShareInfoChanged,
        this,
        [this](mega::MegaHandle handle)
        {
            emit incomingShareAccessChanged();

            const auto rootIndex = getCurrentRootIndex();
            if (!rootIndex.isValid())
            {
                return;
            }

            if (getHandleByIndex(rootIndex) == handle)
            {
                notifyViewStateChanged();
                return;
            }

            const auto incomingShareIndex = getParentIncomingShareByIndex(rootIndex);
            if (incomingShareIndex.isValid() && getHandleByIndex(incomingShareIndex) == handle)
            {
                notifyViewStateChanged();
            }
        },
        Qt::UniqueConnection);
}

QString NodeSelectorTreeViewWidgetIncomingShares::getRootText() const
{
    return MegaNodeNames::getIncomingSharesName();
}

std::unique_ptr<NodeSelectorModel> NodeSelectorTreeViewWidgetIncomingShares::createModel()
{
    return std::unique_ptr<NodeSelectorModelIncomingShares>(new NodeSelectorModelIncomingShares);
}

bool NodeSelectorTreeViewWidgetIncomingShares::isCurrentRootIndexReadOnly() const
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

bool NodeSelectorTreeViewWidgetIncomingShares::isNewFolderAllowed()
{
    // No navigation in file pickers: a folder can only be created inside a single selected folder
    // with at least read-write access (read-write or full access).
    auto selection(ui->tMegaFolders->selectedRows());
    if (selection.size() != 1)
    {
        return false;
    }

    auto node(mProxyModel->getNode(selection.first()));
    return node && node->isFolder() &&
           MegaSyncApp->getMegaApi()->getAccess(node.get()) >= mega::MegaShare::ACCESS_READWRITE;
}

SelectType::EmptyPageInfo NodeSelectorTreeViewWidgetIncomingShares::getEmptyRootPageInfo()
{
    SelectType::EmptyPageInfo info;

    info.title = tr("No incoming shares");
    info.description = tr("Folders shared with you will appear here");
    info.icon.addFile(Utilities::getPixmapName(QLatin1String("empty_incoming_shares"),
                                               Utilities::AttributeType::NONE));

    return info;
}

/////////////////////////////////////////////////////////////////
NodeSelectorTreeViewWidgetBackups::NodeSelectorTreeViewWidgetBackups(SelectTypeSPtr mode,
                                                                     QWidget* parent):
    NodeSelectorTreeViewWidget(mode, TabItem::BACKUPS, parent)
{
    // Monitor Device Names changes and update the title if the current folder is a Device Folder
    auto deviceNamesRequest = UserAttributes::DeviceNames::requestDeviceNames();
    connect(deviceNamesRequest.get(),
            &UserAttributes::DeviceNames::attributeReady,
            this,
            [&]()
            {
                auto rootIndex(ui->tMegaFolders->rootIndex());
                auto* item = NodeSelectorModel::getItemByIndex(rootIndex);
                if (item && item->isDeviceFolder())
                {
                    notifyViewStateChanged();
                }
            });
}

QString NodeSelectorTreeViewWidgetBackups::getRootText() const
{
    return MegaNodeNames::getBackupsName();
}

std::unique_ptr<NodeSelectorModel> NodeSelectorTreeViewWidgetBackups::createModel()
{
    return std::unique_ptr<NodeSelectorModelBackups>(new NodeSelectorModelBackups);
}

SelectType::EmptyPageInfo NodeSelectorTreeViewWidgetBackups::getEmptyRootPageInfo()
{
    SelectType::EmptyPageInfo info;
    info.title = tr("No backups");
    info.icon.addFile(
        Utilities::getPixmapName(QLatin1String("empty_backups"), Utilities::AttributeType::NONE));
    info.buttons = SelectType::ButtonId::ADD_BACKUP;
    return info;
}

/////////////////////////////////////////////////////////////////

NodeSelectorTreeViewWidgetSearch::NodeSelectorTreeViewWidgetSearch(SelectTypeSPtr mode,
                                                                   QWidget* parent):
    NodeSelectorTreeViewWidget(mode, TabItem::SEARCH, parent)
{
    mSearchController = std::make_unique<NodeSelectorSearchController>(ui);

    connect(ui->cloudDriveSearch,
            &TabSelector::clicked,
            this,
            [this]()
            {
                onSearchTabClicked(TabType::CLOUD_DRIVE);
            });

    connect(ui->incomingSharesSearch,
            &TabSelector::clicked,
            this,
            [this]()
            {
                onSearchTabClicked(TabType::INCOMING_SHARE);
            });

    connect(ui->backupsSearch,
            &TabSelector::clicked,
            this,
            [this]()
            {
                onSearchTabClicked(TabType::BACKUP);
            });

    connect(ui->rubbishSearch,
            &TabSelector::clicked,
            this,
            [this]()
            {
                onSearchTabClicked(TabType::RUBBISH);
            });

    ui->tMegaFolders->loadingView().setDelayTimeToShowInMs(0);

    // Search results are always fully expanded (expandSearchResults), so skip the per-node
    // expanded-state save/restore across the loading-scene model detach and use a single
    // batched expandAll() instead. Avoids O(N) collect + O(N^2) setExpanded on every chip switch.
    ui->tMegaFolders->setAutoExpandAll(true);
}

void NodeSelectorTreeViewWidgetSearch::prepareForInitialDisplay()
{
    if (mSelectType->isFilePicker())
    {
        // The search view does not load content on startup. Pre-attaching the view avoids the
        // first visible search having to build the tree view and delegates mid-transition.
        setLoadingSceneVisible(false);
        onLevelLoaded();
    }
}

void NodeSelectorTreeViewWidgetSearch::setSearchScope(std::optional<TabType> scope)
{
    mSearchController->setSearchScope(scope);
}

void NodeSelectorTreeViewWidgetSearch::resetSearchState()
{
    if (auto model = searchModel())
    {
        model->stopSearch();
    }

    mSearchController->resetSearch();
}

void NodeSelectorTreeViewWidgetSearch::search(const QString& text)
{
    // The search destroys the model items in the worker thread. Clear the selection
    // now (while the items are still alive) so no dangling ranges remain that would
    // later dereference freed memory. The model reset also clears it (see the
    // modelAboutToBeReset connection); this is the first line of defense.
    clearSelection();

    mSearchController->beginSearch(text);
    setStyleSheet(styleSheet());

    if (mSearchDelegate)
    {
        mSearchDelegate->setSearchText(text);
    }

    if (auto model = searchModel())
    {
        model->setAllowedTabTypes(
            mSearchController->allowedTypes(getSelectType()->allowedTabTypes()));
        model->searchByText(text);
    }
}

void NodeSelectorTreeViewWidgetSearch::stopSearch()
{
    if (auto model = searchModel())
    {
        model->stopSearch();
    }
    mSearchController->stopSearch();
}

std::shared_ptr<NodeSelectorProxyModel> NodeSelectorTreeViewWidgetSearch::createProxyModel()
{
    auto proxy =
        std::make_shared<NodeSelectorProxyModelSearch>(getSelectType()->createProxyModel());

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

bool NodeSelectorTreeViewWidgetSearch::isCurrentRootIndexReadOnly() const
{
    return true;
}

bool NodeSelectorTreeViewWidgetSearch::isSelectionReadOnly(const QModelIndexList& selection)
{
    return true;
}

void NodeSelectorTreeViewWidgetSearch::resetMovingNumber()
{
    mModel->finishMovingNodes();
}

bool NodeSelectorTreeViewWidgetSearch::isNodeCompatibleWithModel(mega::MegaNode* node)
{
    return mSearchController->matchesNodeName(node);
}

void NodeSelectorTreeViewWidgetSearch::makeViewConnections()
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

    if (mSearchController->hasRows() && node)
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

void NodeSelectorTreeViewWidgetSearch::onSearchTabClicked(TabType type)
{
    mSearchController->activateMode(
        type,
        searchProxyModel(),
        [this](TabType t)
        {
            changeColumnsVisibility(t);
        },
        [this]()
        {
            expandSearchResults();
        });
}

void NodeSelectorTreeViewWidgetSearch::onItemDoubleClick(const QModelIndex& index)
{
    auto node = qvariant_cast<std::shared_ptr<MegaNode>>(
        index.data(toInt(NodeSelectorModelRoles::NODE_ROLE)));
    emit nodeDoubleClicked(node, true);
}

void NodeSelectorTreeViewWidgetSearch::changeColumnsVisibility(TabType type)
{
    emit searchTabTypeChanged(type);
}

void NodeSelectorTreeViewWidgetSearch::expandSearchResults()
{
    // Expand once the data is ready; the view defers it if the model is detached.
    ui->tMegaFolders->expandAllWhenReady();
}

void NodeSelectorTreeViewWidgetSearch::onLevelLoaded()
{
    mSearchController->handleExpandReady(
        searchModel(),
        searchProxyModel(),
        ui->tMegaFolders->model() != nullptr,
        [this]()
        {
            NodeSelectorTreeViewWidget::onLevelLoaded();
        },
        [this](TabType type)
        {
            changeColumnsVisibility(type);
        },
        [this]()
        {
            expandSearchResults();
        });
}

QString NodeSelectorTreeViewWidgetSearch::getRootText() const
{
    const auto count = searchModel() ? searchModel()->searchResultCount() : 0;
    return tr("%n result found", "", count);
}

std::unique_ptr<NodeSelectorModel> NodeSelectorTreeViewWidgetSearch::createModel()
{
    auto model = std::unique_ptr<NodeSelectorModelSearch>(
        new NodeSelectorModelSearch(getSelectType()->allowedTabTypes(),
                                    getSelectType()->flattenSearchResults()));

    connect(model.get(),
            &NodeSelectorModelSearch::nodeTypeHasChanged,
            this,
            &NodeSelectorTreeViewWidgetSearch::setViewPage);

    mRestoreManager = std::make_shared<RestoreNodeManager>(model.get(), this);

    // Detect if the row count changed
    connect(model.get(),
            &QAbstractItemModel::rowsInserted,
            this,
            &NodeSelectorTreeViewWidget::notifyViewStateChanged);

    connect(model.get(),
            &QAbstractItemModel::rowsRemoved,
            this,
            &NodeSelectorTreeViewWidget::notifyViewStateChanged);

    connect(model.get(),
            &QAbstractItemModel::modelReset,
            this,
            &NodeSelectorTreeViewWidget::notifyViewStateChanged);

    connect(model.get(),
            &QAbstractItemModel::rowsInserted,
            this,
            &NodeSelectorTreeViewWidgetSearch::searchCounterChanged);
    connect(model.get(),
            &QAbstractItemModel::rowsRemoved,
            this,
            &NodeSelectorTreeViewWidgetSearch::searchCounterChanged);
    connect(model.get(),
            &QAbstractItemModel::modelReset,
            this,
            &NodeSelectorTreeViewWidgetSearch::searchCounterChanged);

    return model;
}

int NodeSelectorTreeViewWidgetSearch::searchResultCount() const
{
    return searchModel() ? searchModel()->searchResultCount() : 0;
}

SelectType::EmptyPageInfo NodeSelectorTreeViewWidgetSearch::getEmptyRootPageInfo()
{
    SelectType::EmptyPageInfo info;
    info.title = tr("No results found");
    info.description = tr("Try a different name or check the spelling");
    info.icon.addFile(
        Utilities::getPixmapName(QLatin1String("empty_search"), Utilities::AttributeType::NONE));
    return info;
}

void NodeSelectorTreeViewWidgetSearch::setViewPage()
{
    if (!mModel)
    {
        return;
    }

    NodeSelectorTreeViewWidget::setViewPage();

    if (ui->tMegaFolders->model())
    {
        const bool hasRows = ui->tMegaFolders->model()->rowCount() > 0;
        mSearchController->setHasRows(hasRows);
        if (!hasRows && showEmptyView())
        {
            setCurrentPage(ViewType::ROOT_EMPTY);
            return;
        }
    }
    setCurrentPage(ViewType::VIEW);
}

NodeSelectorModelSearch* NodeSelectorTreeViewWidgetSearch::searchModel() const
{
    return static_cast<NodeSelectorModelSearch*>(mModel.get());
}

NodeSelectorProxyModelSearch* NodeSelectorTreeViewWidgetSearch::searchProxyModel() const
{
    return static_cast<NodeSelectorProxyModelSearch*>(mProxyModel.get());
}

NodeSelectorDelegate* NodeSelectorTreeViewWidgetSearch::createItemDelegate(QObject* parent)
{
    auto delegate = new NodeSearchRowDelegate(parent);
    // The first search runs before the view exists, so the setSearchText() call in search()
    // finds no delegate yet. Apply the active search text now that the delegate is created,
    // otherwise the first search results are painted without the match highlight.
    delegate->setSearchText(mSearchController->searchText());
    mSearchDelegate = delegate;
    return delegate;
}

///////////////////////
NodeSelectorTreeViewWidgetRubbish::NodeSelectorTreeViewWidgetRubbish(SelectTypeSPtr mode,
                                                                     QWidget* parent):
    NodeSelectorTreeViewWidget(mode, TabItem::RUBBISH, parent)
{}

void NodeSelectorTreeViewWidgetRubbish::setShowEmptyView(bool newShowEmptyView)
{
    mShowEmptyView = newShowEmptyView;
}

bool NodeSelectorTreeViewWidgetRubbish::isTopRootEmpty() const
{
    auto rootIndex = mModel->index(0, 0);
    return mModel->rowCount(rootIndex) == 0;
}

bool NodeSelectorTreeViewWidgetRubbish::isNodeCompatibleWithModel(mega::MegaNode* node)
{
    return MegaSyncApp->getMegaApi()->isInRubbish(node);
}

void NodeSelectorTreeViewWidgetRubbish::makeViewConnections()
{
    connect(ui->tMegaFolders,
            &NodeSelectorTreeView::restoreClicked,
            mRestoreManager.get(),
            &RestoreNodeManager::onRestoreClicked);
}

QString NodeSelectorTreeViewWidgetRubbish::getRootText() const
{
    return MegaNodeNames::getRubbishName();
}

std::unique_ptr<NodeSelectorModel> NodeSelectorTreeViewWidgetRubbish::createModel()
{
    auto model = std::unique_ptr<NodeSelectorModelRubbish>(new NodeSelectorModelRubbish);

    mRestoreManager = std::make_shared<RestoreNodeManager>(model.get(), this);

    return model;
}

void NodeSelectorTreeViewWidgetRubbish::setViewPage()
{
    auto rootIndex = mModel->index(0, 0);
    if (mModel->rowCount(rootIndex) == 0 && showEmptyView())
    {
        setCurrentPage(ViewType::ROOT_EMPTY);

        // The rubbish has been emptied, so we can unset the loading view. If a proxy job
        // is still running, the loading scene ignores this request (isWorking()) and the
        // job's finished handler hides it instead.
        setLoadingSceneVisible(false);
    }
    else
    {
        NodeSelectorTreeViewWidget::setViewPage();
    }
}

SelectType::EmptyPageInfo NodeSelectorTreeViewWidgetRubbish::getEmptyRootPageInfo()
{
    SelectType::EmptyPageInfo info;
    info.title = tr("Rubbish bin is empty");
    info.icon.addFile(
        Utilities::getPixmapName(QLatin1String("empty_rubbish"), Utilities::AttributeType::NONE));

    return info;
}
