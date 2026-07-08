#include "NodeSelectorTreeViewWidget.h"

#include "EventUpdater.h"
#include "MegaApplication.h"
#include "MegaNodeNames.h"
#include "NodeSelectorDelegates.h"
#include "NodeSelectorModel.h"
#include "NodeSelectorProxyModel.h"
#include "NodeSelectorSelectionCoordinator.h"
#include "NodeSelectorTreeView.h"
#include "NodeSelectorTreeViewWidgetSpecializations.h"
#include "RequestListenerManager.h"
#include "TokenizableItems/TokenPropertySetter.h"
#include "ui_NodeSelectorTreeViewWidget.h"

#include <QGuiApplication>
#include <QKeyEvent>
#include <QLayout>
#include <QScrollBar>

#include <algorithm>

const int CHECK_UPDATED_NODES_INTERVAL = 1000;
const int IMMEDIATE_CHECK_UPDATES_NODES_THRESHOLD = 200;
// Coalescing window for view-state refreshes on model row changes. Keep in sync with
// SEARCH_PATH_ITEMS_RESORT_DEBOUNCE_MS (NodeSelectorModelSpecialised.cpp): both coalesce
// the two halves of the same node-update storm.
const int VIEW_REFRESH_DEBOUNCE_MS = 100;

NodeSelectorTreeViewWidget::NodeSelectorTreeViewWidget(SelectTypeSPtr mode,
                                                       TabItem tabType,
                                                       QWidget* parent):
    QWidget(parent),
    ui(new Ui::NodeSelectorTreeViewWidget),
    mProxyModel(nullptr),
    mModel(nullptr),
    mNodeActions(MegaSyncApp->getMegaApi()),
    mMegaApi(MegaSyncApp->getMegaApi()),
    mSelectType(mode),
    mManuallyResizedColumn(false),
    mShowLabelText(true),
    mResizeEventsReceived(0),
    first(true),
    mUiBlocked(false),
    mWasEmpty(true),
    mTabType(tabType)
{
    ui->setupUi(this);
    setFocusProxy(ui->tMegaFolders);
    ui->searchButtonsWidget->setVisible(false);

    connect(&ui->tMegaFolders->loadingView(),
            &ViewLoadingSceneBase::sceneVisibilityChange,
            this,
            &NodeSelectorTreeViewWidget::onUiBlocked);

    // Chip selector configuration
    BaseTokens iconTokens;
    iconTokens.setNormalOff(QLatin1String("icon-primary"));
    iconTokens.setNormalOn(QLatin1String("brand-on-container"));
    auto iconTokensSetter = std::make_shared<TokenPropertySetter>(iconTokens);
    TabSelector::applyTokens(ui->searchButtonsWidget, iconTokensSetter);

    // Set search tabs titles
    ui->cloudDriveSearch->setProperty("title", MegaNodeNames::getCloudDriveName());
    ui->backupsSearch->setProperty("title", MegaNodeNames::getBackupsName());
    ui->incomingSharesSearch->setProperty("title", MegaNodeNames::getIncomingSharesName());
    ui->rubbishSearch->setProperty("title", MegaNodeNames::getRubbishName());

    mResizeEventsTimer.setSingleShot(true);
    mResizeEventsTimer.setInterval(10);
    connect(&mResizeEventsTimer,
            &QTimer::timeout,
            this,
            [this]()
            {
                if (mResizeEventsReceived > 3)
                {
                    mManuallyResizedColumn = true;
                }

                mResizeEventsReceived = 0;
            });

    // A proxy refilter emits one rowsInserted per contiguous range (~1600 on a search chip
    // switch), and bulk node updates from other clients arrive as long bursts of add/remove
    // events. Refreshing the page/header/breadcrumb per event froze the UI, so the requests
    // are coalesced: the first one arms the timer and the refresh runs once when it fires,
    // with the final model state (at most one execution per interval during a sustained
    // stream).
    mCheckViewOnModelChangeDebounce.setSingleShot(true);
    mCheckViewOnModelChangeDebounce.setInterval(VIEW_REFRESH_DEBOUNCE_MS);
    connect(&mCheckViewOnModelChangeDebounce,
            &QTimer::timeout,
            this,
            &NodeSelectorTreeViewWidget::executeCheckViewOnModelChange);

    // Empty pages
    ui->emptyPage->installEventFilter(this);
    ui->emptyPage->setFocusPolicy(Qt::StrongFocus);
    ui->emptyPage->setAcceptDrops(false);

    auto emitUpload = [this]()
    {
        emit onCustomButtonClicked(SelectType::ButtonId::UPLOAD);
    };
    auto emitNewFolder = [this]()
    {
        emit onCustomButtonClicked(SelectType::ButtonId::NEW_FOLDER);
    };
    auto emitAddBackup = [this]()
    {
        emit onCustomButtonClicked(SelectType::ButtonId::ADD_BACKUP);
    };
    connect(ui->bEmptyUpload, &QPushButton::clicked, this, emitUpload);
    connect(ui->bEmptyNewFolder, &QPushButton::clicked, this, emitNewFolder);
    connect(ui->bEmptyAddBackup, &QPushButton::clicked, this, emitAddBackup);

    mNodeActions.setDialogParent(Utilities::getTopParent<QDialog>(ui->tMegaFolders));

    setupHeaderDivider();
}

NodeSelectorTreeViewWidget::~NodeSelectorTreeViewWidget()
{
    delete ui;
}

void NodeSelectorTreeViewWidget::init()
{
    // When init, show the loading view and then, add a 150 delay to avoid showing the loading view
    // for short loads
    setLoadingSceneVisible(true);
    ui->tMegaFolders->loadingView().setDelayTimeToShowInMs(150);
    mProxyModel = createProxyModel();
    mModel = createModel();
    mSelectType->initTreeViewWidget(this);
    mSelectionCoordinator = std::make_unique<NodeSelectorSelectionCoordinator>(
        mMegaApi,
        NodeSelectorSelectionCoordinator::Objects{
            mModel.get(),
            mProxyModel.get(),
            ui->tMegaFolders,
        },
        NodeSelectorSelectionCoordinator::Policy{
            [this](const QModelIndex& index, bool setCurrent, bool exclusiveSelect)
            {
                selectIndex(index, setCurrent, exclusiveSelect);
            },
            [this]()
            {
                clearSelection();
            },
            [this](const QModelIndex& index)
            {
                onItemDoubleClick(index);
            },
            [this](const QModelIndex& index)
            {
                // File pickers cannot navigate into folders, so a freshly created folder must be
                // selected directly. File-manager mode keeps navigating into it.
                if (mSelectType->isFilePicker())
                {
                    selectIndex(index, true, true);
                }
                else
                {
                    onItemDoubleClick(index);
                }
            },
            [this]()
            {
                setRootIndex(mProxyModel->getTopRootIndex());
            },
            [this](std::function<bool()> selectionOperation)
            {
                disconnect(ui->tMegaFolders->selectionModel(),
                           &QItemSelectionModel::selectionChanged,
                           this,
                           &NodeSelectorTreeViewWidget::onSelectionChanged);
                const bool shouldNotify = selectionOperation();
                connect(ui->tMegaFolders->selectionModel(),
                        &QItemSelectionModel::selectionChanged,
                        this,
                        &NodeSelectorTreeViewWidget::onSelectionChanged);
                if (shouldNotify)
                {
                    onSelectionChanged(QItemSelection(), QItemSelection());
                }
            },
        });

    auto updateObjects = NodeSelectorModelUpdateCoordinator::Objects{
        mModel.get(),
        mProxyModel.get(),
        ui->tMegaFolders,
    };

    auto updateSharedState = NodeSelectorModelUpdateCoordinator::SharedState{
        mSelectionCoordinator->movedHandlesToSelect(),
        mSelectionCoordinator->parentOfRestoredNodes(),
        mSelectionCoordinator->mergeTargetFolders(),
        mNodesToBeReplaced,
    };

    auto updatePolicy = NodeSelectorModelUpdateCoordinator::Policy{
        [this](const QModelIndex& index, mega::MegaNode* node)
        {
            return static_cast<int>(getNodeOnModelState(index, node));
        },
        [this](mega::MegaHandle parentHandle)
        {
            return getAddedNodeParent(parentHandle);
        },
    };

    mModelUpdateCoordinator =
        std::make_unique<NodeSelectorModelUpdateCoordinator>(mMegaApi,
                                                             updateObjects,
                                                             updateSharedState,
                                                             std::move(updatePolicy));

    connect(mModelUpdateCoordinator.get(),
            &NodeSelectorModelUpdateCoordinator::indexRemovedAffectingCurrentPath,
            this,
            &NodeSelectorTreeViewWidget::onRemovedIndexAffectsCurrentRoot);
    connect(mModelUpdateCoordinator.get(),
            &NodeSelectorModelUpdateCoordinator::nodesRenamed,
            this,
            &NodeSelectorTreeViewWidget::nodesRenamed);
    // A root node removed while the user is inside it would leave the open root dangling.
    // Reuse the same navigate-away path, mapping the model's source index to the proxy.
    connect(mModel.get(),
            &NodeSelectorModel::rootNodeAboutToBeRemoved,
            this,
            [this](const QModelIndex& sourceIndex)
            {
                onRemovedIndexAffectsCurrentRoot(mProxyModel->mapFromSource(sourceIndex));
            });
    connect(mModelUpdateCoordinator.get(),
            &NodeSelectorModelUpdateCoordinator::viewStateChanged,
            this,
            &NodeSelectorTreeViewWidget::notifyViewStateChanged);

    mNodeActions.setModel(mModel.get());

    enableDragAndDrop(mSelectType->acceptDrops(mTabType));
    mModel->setExtraSpaceEnabled(!mSelectType->isFilePicker());

    // Do not use QTreeView::setSortingEnabled(): with it enabled, QTreeView::setModel() and
    // QHeaderView::restoreState() re-trigger model->sort() while the loading scene is
    // reattaching the view. At that point the previous QFutureWatcher already reports
    // finished, so a new concurrent sort job starts and mutates the proxy mapping while the
    // reattach walks it (setSelectionModel -> QHeaderView::currentChanged) -> crash.
    // Replicate the same UX (clickable header + sort indicator) and route genuine indicator
    // changes to the proxy explicitly instead.
    ui->tMegaFolders->header()->setSortIndicatorShown(true);
    ui->tMegaFolders->header()->setSectionsClickable(true);
    connect(ui->tMegaFolders->header(),
            &QHeaderView::sortIndicatorChanged,
            mProxyModel.get(),
            &NodeSelectorProxyModel::onSortIndicatorChanged);
    ui->tMegaFolders->viewport()->installEventFilter(this);

    mProxyModel->setSourceModel(mModel.get());

    connect(mProxyModel.get(),
            &NodeSelectorProxyModel::levelLoaded,
            this,
            &NodeSelectorTreeViewWidget::onLevelLoaded);
    connect(mProxyModel.get(),
            &NodeSelectorProxyModel::modelSorted,
            this,
            &NodeSelectorTreeViewWidget::onModelRowsChanged);
    connect(mModel.get(),
            &QAbstractItemModel::rowsInserted,
            this,
            &NodeSelectorTreeViewWidget::onModelRowsChanged);
    connect(mModel.get(),
            &QAbstractItemModel::rowsRemoved,
            this,
            &NodeSelectorTreeViewWidget::onModelRowsChanged);
    // The proxy sorts asynchronously: layoutAboutToBeChanged() is emitted before the
    // selection of a moved node is applied, and the matching layoutChanged() arrives
    // afterwards, so QItemSelectionModel rebuilds the selection from a snapshot that
    // predates it and drops the moved node. Re-select the pending moved handles AFTER
    // layoutChanged is fully processed (queued -> runs once the selection model has
    // already applied its clobbering rebuild).
    connect(mProxyModel.get(),
            &QAbstractItemModel::layoutChanged,
            mSelectionCoordinator.get(),
            &NodeSelectorSelectionCoordinator::reapplyMovedSelection,
            Qt::QueuedConnection);
    connect(mModel.get(),
            &NodeSelectorModel::blockUi,
            this,
            &NodeSelectorTreeViewWidget::setLoadingSceneVisible);
    // Re-emit when the model commits the current root (possibly deferred until children
    // load), so the navigation breadcrumb refreshes against the committed root.
    connect(mModel.get(),
            &NodeSelectorModel::currentRootIndexChanged,
            this,
            &NodeSelectorTreeViewWidget::rootIndexChanged);
    connect(mModel.get(),
            &NodeSelectorModel::modelModified,
            this,
            &NodeSelectorTreeViewWidget::onModelModified);
    connect(mModel.get(),
            &NodeSelectorModel::nodesRenamed,
            this,
            &NodeSelectorTreeViewWidget::nodesRenamed);
    connect(mModel.get(),
            &NodeSelectorModel::itemsMoved,
            mSelectionCoordinator.get(),
            &NodeSelectorSelectionCoordinator::onItemsMoved);
    connect(mModel.get(),
            &NodeSelectorModel::nodesAdded,
            mSelectionCoordinator.get(),
            &NodeSelectorSelectionCoordinator::onNodesAdded);

#ifdef __APPLE__
    ui->tMegaFolders->setAnimated(false);
#endif

    connect(&mNodesUpdateTimer,
            &QTimer::timeout,
            this,
            &NodeSelectorTreeViewWidget::processCachedNodesUpdated);
    mNodesUpdateTimer.start(CHECK_UPDATED_NODES_INTERVAL);
}

void NodeSelectorTreeViewWidget::initEmptyRootPageMessages()
{
    showRootEmptyState();
}

void NodeSelectorTreeViewWidget::initEmptyFolderMessages()
{
    showFolderEmptyState();
}

void NodeSelectorTreeViewWidget::showRootEmptyState()
{
    applyEmptyState(getEmptyRootPageInfo(), ViewType::ROOT_EMPTY);
}

void NodeSelectorTreeViewWidget::showFolderEmptyState()
{
    if (mSelectType)
    {
        applyEmptyState(getEmptyFolderPageInfo(), ViewType::FOLDER_EMPTY);
    }
}

void NodeSelectorTreeViewWidget::setCurrentPage(ViewType type)
{
    const auto previousType = mCurrentViewType;

    // We still don´t know if the view is empty as it is still loading it
    if ((type == ViewType::ROOT_EMPTY || type == ViewType::FOLDER_EMPTY) &&
        ui->tMegaFolders->loadingView().isLoadingViewSet())
    {
        return;
    }

    switch (type)
    {
        case ViewType::ROOT_EMPTY:
        {
            mWasEmpty = true;
            mRootWasEmpty = true;
            showRootEmptyState();
            ui->stackedWidget->setCurrentWidget(ui->emptyPage);
            break;
        }
        case ViewType::FOLDER_EMPTY:
        {
            mWasEmpty = true;
            mRootWasEmpty = true;
            showFolderEmptyState();
            ui->stackedWidget->setCurrentWidget(ui->emptyPage);
            break;
        }
        case ViewType::VIEW:
        default:
        {
            mWasEmpty = false;
            mRootWasEmpty = false;
            mCurrentViewType = ViewType::VIEW;
            ui->stackedWidget->setCurrentWidget(ui->treeViewPage);
            break;
        }
    }

    if (ui->stackedWidget->currentWidget() != ui->treeViewPage)
    {
        ui->stackedWidget->currentWidget()->setFocus(Qt::OtherFocusReason);
    }
    else
    {
        ui->tMegaFolders->setFocus(Qt::OtherFocusReason);
    }

    if (previousType != mCurrentViewType)
    {
        emit currentViewPageChanged(mCurrentViewType);
    }
}

void NodeSelectorTreeViewWidget::applyEmptyState(const SelectType::EmptyPageInfo& info,
                                                 ViewType type)
{
    mCurrentViewType = type;
    const bool isEmptyFolderState = type == ViewType::FOLDER_EMPTY;

    ui->descriptionEmptyLabel->hide();
    ui->titleEmptyLabel->hide();

    if (!info.description.isEmpty())
    {
        ui->descriptionEmptyLabel->setText(info.description);
        ui->descriptionEmptyLabel->show();
    }
    if (!info.title.isEmpty())
    {
        ui->titleEmptyLabel->setText(info.title);
        ui->titleEmptyLabel->show();
    }
    if (!info.icon.isNull())
    {
        ui->emptyIcon->setIcon(info.icon);
    }

    static const char* HAS_BORDER_PROPERTY("hasBorder");
    ui->frame->setProperty(HAS_BORDER_PROPERTY, info.hasBorder);
    ui->gridLayout->setRowStretch(0, 1);
    const bool ADD_MARGINS{isEmptyFolderState || info.hasBorder};
    ui->gridLayout->setRowStretch(4, ADD_MARGINS ? 1 : 4);
    setStyleSheet(styleSheet());
    setEmptyStateButtonsVisibility(info);
}

void NodeSelectorTreeViewWidget::setEmptyStateButtonsVisibility(
    const SelectType::EmptyPageInfo& info)
{
    const bool showUpload = mSelectType && info.buttons.testFlag(SelectType::ButtonId::UPLOAD) &&
                            mSelectType->showEmptyStateUploadButton(this);
    const bool showNewFolder = mSelectType &&
                               info.buttons.testFlag(SelectType::ButtonId::NEW_FOLDER) &&
                               mSelectType->showEmptyStateNewFolderButton(this);
    const bool showAddBackup = mSelectType &&
                               info.buttons.testFlag(SelectType::ButtonId::ADD_BACKUP) &&
                               mSelectType->showEmptyStateAddBackupButton(this);

    ui->bEmptyUpload->setVisible(showUpload);
    ui->bEmptyNewFolder->setVisible(showNewFolder);
    ui->bEmptyAddBackup->setVisible(showAddBackup);
    ui->emptyPageButtonsWidget->setVisible(showUpload || showNewFolder || showAddBackup);
}

bool NodeSelectorTreeViewWidget::event(QEvent* event)
{
    if (event->type() == QEvent::LanguageChange)
    {
        ui->retranslateUi(this);

        if (mCurrentViewType == ViewType::FOLDER_EMPTY)
        {
            showFolderEmptyState();
        }
        else if (mCurrentViewType == ViewType::ROOT_EMPTY)
        {
            showRootEmptyState();
        }
    }
    else if (event->type() == QEvent::MouseButtonRelease)
    {
        clearSelection();
    }
    else if (event->type() == QEvent::Resize)
    {
        updateHeaderDividerGeometry();
    }

    return QWidget::event(event);
}

bool NodeSelectorTreeViewWidget::eventFilter(QObject* watched, QEvent* event)
{
#ifndef Q_OS_MACOS
    if (watched == ui->tMegaFolders->verticalScrollBar() &&
        (event->type() == QEvent::Show || event->type() == QEvent::Hide))
    {
        updateHeaderDividerGeometry();
        return QWidget::eventFilter(watched, event);
    }
#endif
    if (event->type() == QEvent::DragEnter)
    {
        if (auto dropEvent = static_cast<QDragEnterEvent*>(event))
        {
            if (!dropEvent->mimeData()->urls().isEmpty())
            {
                ui->tMegaFolders->dragEnterEvent(dropEvent);
            }
            else
            {
                // On the empty page the drop target is the current folder; on the tree it is
                // the hovered item.
                auto sourceIndex =
                    (ui->stackedWidget->currentWidget() != ui->treeViewPage) ?
                        mModel->getCurrentRootIndex() :
                        mProxyModel->mapToSource(ui->tMegaFolders->indexAt(dropEvent->pos()));

                if (mModel->canDropMimeData(dropEvent->mimeData(),
                                            Qt::MoveAction,
                                            sourceIndex.row(),
                                            sourceIndex.column(),
                                            sourceIndex.parent()))
                {
                    dropEvent->acceptProposedAction();
                }
            }
        }
    }
    else if (event->type() == QEvent::DragMove)
    {
        if (auto moveEvent = static_cast<QDragMoveEvent*>(event))
        {
            if (!moveEvent->mimeData()->urls().isEmpty())
            {
                ui->tMegaFolders->dragMoveEvent(moveEvent);
            }
            else if (ui->stackedWidget->currentWidget() != ui->treeViewPage)
            {
                // Internal move over the empty page: the target is the current folder
                // (there is no tree item to delegate the drag to).
                auto target = mModel->getCurrentRootIndex();
                if (mModel->canDropMimeData(moveEvent->mimeData(),
                                            Qt::MoveAction,
                                            target.row(),
                                            target.column(),
                                            target.parent()))
                {
                    moveEvent->acceptProposedAction();
                }
            }
        }
    }
    else if (event->type() == QEvent::Drop &&
             ui->stackedWidget->currentWidget() != ui->treeViewPage)
    {
        dropIntoRootIndex(static_cast<QDropEvent*>(event));
    }
    else if (event->type() == QEvent::Resize)
    {
        if (watched == ui->tMegaFolders->viewport())
        {
            updateColumnsWidth(false);
            updateHeaderDividerGeometry();
        }
    }
    // Propagate key events to the view
    else if (watched == ui->emptyPage &&
             (event->type() == QEvent::ShortcutOverride || event->type() == QEvent::KeyPress))
    {
        if (auto keyEvent = static_cast<QKeyEvent*>(event); keyEvent->matches(QKeySequence::Paste))
        {
            // Our way to
            if (mSelectType->areActionsAllowed())
            {
                if (event->type() == QEvent::ShortcutOverride)
                {
                    event->accept();
                }
                else
                {
                    QMetaObject::invokeMethod(ui->tMegaFolders,
                                              "onPasteShortcutActivated",
                                              Qt::DirectConnection);
                }
            }

            return true;
        }
    }
    else if (watched == ui->emptyPage && event->type() == QEvent::ContextMenu)
    {
        ui->tMegaFolders->contextMenuEvent(static_cast<QContextMenuEvent*>(event));
    }

    return QWidget::eventFilter(watched, event);
}

bool NodeSelectorTreeViewWidget::clearSelection()
{
    auto hasSelection{!ui->tMegaFolders->selectedRows().isEmpty()};
    ui->tMegaFolders->clearSelection();
    return hasSelection;
}

void NodeSelectorTreeViewWidget::abort()
{
    mModel->abort();
}

void NodeSelectorTreeViewWidget::moveToTopRootIndex()
{
    setRootIndex(mProxyModel->getTopRootIndex());
}

NodeSelectorModelItem* NodeSelectorTreeViewWidget::rootItem()
{
    auto rootIndex = ui->tMegaFolders->rootIndex();
    if (!rootIndex.isValid())
    {
        // Top parent
        rootIndex = mModel->index(0, 0, QModelIndex());
    }

    return mModel->getItemByIndex(rootIndex);
}

QModelIndex NodeSelectorTreeViewWidget::getCurrentRootIndex() const
{
    // The model's committed root mapped to the proxy. It is the single source of truth for the
    // current folder and, unlike the view's rootIndex(), is not transiently invalidated by the
    // loading-scene detach.
    return (mProxyModel && mModel) ? mProxyModel->mapFromSource(mModel->getCurrentRootIndex()) :
                                     QModelIndex();
}

NodeSelectorProxyModel* NodeSelectorTreeViewWidget::getProxyModel()
{
    return mProxyModel.get();
}

void NodeSelectorTreeViewWidget::showCurrentRootContextMenu(const QPoint& globalPos)
{
    if (mUiBlocked)
    {
        return;
    }

    // The chevron acts on the folder we are currently inside: a single index (the current root),
    // handled like a click on empty space so folder-level actions are offered.
    ui->tMegaFolders->clearSelection();
    ui->tMegaFolders->showContextMenu(QModelIndexList(),
                                      getCurrentRootIndex(),
                                      true,
                                      globalPos,
                                      true);
}

QList<NodeSelectorBreadcrumbSegment>
    NodeSelectorTreeViewWidget::navigationBreadcrumbSegments() const
{
    if (!mProxyModel)
    {
        return {};
    }

    // Use the model's authoritative current root mapped to the proxy, not the view's
    // rootIndex(): the latter is transiently reset to invalid while the loading scene
    // detaches the model, which would collapse the breadcrumb to just the root.
    const auto currentRoot = mProxyModel->mapFromSource(mModel->getCurrentRootIndex());

    if (!currentRoot.isValid())
    {
        return {{mega::INVALID_HANDLE, getRootText()}};
    }

    QList<NodeSelectorBreadcrumbSegment> segments;
    for (QModelIndex index = currentRoot; index.isValid(); index = index.parent())
    {
        auto node = mProxyModel->getNode(index);
        const auto handle = node ? node->getHandle() : mega::INVALID_HANDLE;
        segments.prepend({handle, index.data(Qt::DisplayRole).toString()});
    }

    if (!mProxyModel->getTopRootIndex().isValid())
    {
        // Synthetic tab root label, not a node.
        segments.prepend({mega::INVALID_HANDLE, getRootText()});
    }

    return segments;
}

bool NodeSelectorTreeViewWidget::navigateToBreadcrumbSegment(int segmentIndex)
{
    if (mUiBlocked)
    {
        return false;
    }

    const auto targetIndex = indexForBreadcrumbSegment(segmentIndex);
    const auto currentRoot = getCurrentRootIndex();

    if (targetIndex == currentRoot || (!targetIndex.isValid() && !currentRoot.isValid()))
    {
        return false;
    }

    setRootIndex(targetIndex);
    onSelectionHasChanged();
    return true;
}

bool NodeSelectorTreeViewWidget::isShowingEmptyPage() const
{
    return ui->stackedWidget->currentWidget() == ui->emptyPage;
}

NodeSelectorTreeViewWidget::ViewType NodeSelectorTreeViewWidget::currentViewPage() const
{
    return mCurrentViewType;
}

bool NodeSelectorTreeViewWidget::isInRootView() const
{
    return !ui->tMegaFolders->rootIndex().isValid();
}

bool NodeSelectorTreeViewWidget::isTopRootEmpty() const
{
    return ui->tMegaFolders->model() ?
               ui->tMegaFolders->model()->rowCount(mProxyModel->getTopRootIndex()) == 0 :
               true;
}

bool NodeSelectorTreeViewWidget::isAtTopRoot() const
{
    return mProxyModel && getCurrentRootIndex() == mProxyModel->getTopRootIndex();
}

void NodeSelectorTreeViewWidget::enableDragAndDrop(bool enable)
{
    ui->emptyPage->setAcceptDrops(enable);
    ui->tMegaFolders->setDragEnabled(enable);
    ui->tMegaFolders->viewport()->setAcceptDrops(enable);
    ui->tMegaFolders->setDropIndicatorShown(enable);
    ui->tMegaFolders->setDragDropMode(enable ? QAbstractItemView::DragDrop :
                                               QAbstractItemView::NoDragDrop);
}

void NodeSelectorTreeViewWidget::setupHeaderDivider()
{
    // headerDivider is declared in the .ui as a free (non-layout) overlay child of treeViewPage,
    // which spans the full widget width, so the separator can reach edge to edge, past the tree
    // view's left/right layout margins. Its position depends on the runtime header height, so it
    // is placed here rather than statically.
    ui->headerDivider->setAttribute(Qt::WA_TransparentForMouseEvents, true);

    connect(ui->tMegaFolders->header(),
            &QHeaderView::geometriesChanged,
            this,
            &NodeSelectorTreeViewWidget::updateHeaderDividerGeometry);

    // The header geometry is not valid during construction; defer the first placement.
    QTimer::singleShot(0, this, &NodeSelectorTreeViewWidget::updateHeaderDividerGeometry);

#ifndef Q_OS_MACOS
    // On Linux the scrollbar show/hide does not always trigger a viewport resize, so install an
    // event filter to catch those transitions and re-run the geometry update explicitly.
    ui->tMegaFolders->verticalScrollBar()->installEventFilter(this);
#endif
}

void NodeSelectorTreeViewWidget::updateHeaderDividerGeometry()
{
    auto* header = ui->tMegaFolders->header();
    const int dividerY = header->mapTo(ui->treeViewPage, QPoint(0, header->height())).y();

    int dividerWidth = ui->treeViewPage->width();

#ifndef Q_OS_MACOS
    // On Linux the vertical scrollbar spans up to the header height, so a full-width divider would
    // be painted on top of the scrollbar. Trim the divider by the scrollbar width while it is
    // visible so the separator stops before the scrollbar.
    auto* verticalScrollBar = ui->tMegaFolders->verticalScrollBar();
    if (verticalScrollBar && verticalScrollBar->isVisible())
    {
        dividerWidth = ui->tMegaFolders->viewport()->width();
    }
#endif

    ui->headerDivider->setGeometry(0, dividerY, dividerWidth, ui->headerDivider->height());
    ui->headerDivider->raise();
}

void NodeSelectorTreeViewWidget::onSectionResized()
{
    if (!mManuallyResizedColumn && ui->tMegaFolders->header()->rect().contains(
                                       ui->tMegaFolders->mapFromGlobal(QCursor::pos())))
    {
        mResizeEventsReceived++;

        // Protect against clicking on the header to show the sort indicator.
        // Only if the event is received 3 times in an span of 10ms, it is a real resize
        if (!mResizeEventsTimer.isActive())
        {
            mResizeEventsTimer.start();
        }
    }
}

void NodeSelectorTreeViewWidget::updateColumnsWidth(bool updateVisibleColumnCounter)
{
    if (!isTreeViewReady())
    {
        return;
    }

    if (updateVisibleColumnCounter)
    {
        rebuildVisibleColumns();
    }

    if (!mVisibleColumns.isEmpty())
    {
        int minWidth(100);
        int labelColumnMinWidth(88);
        int labelColumnMaxWidth(120);
        int compactLabelColumnWidth(16);
        int maxSecondaryColumnWidth(200);
        double secondaryColumnProportion(0.2);
        double labelColumnProportion(0.12);

        for (QList<int>::const_reverse_iterator column = mVisibleColumns.crbegin();
             column != mVisibleColumns.crend();
             ++column)
        {
            // NODE stretches to fill the remaining space (QHeaderView::Stretch); never size it
            // manually here or it would fight the header's stretch logic.
            if ((*column) == NodeSelectorModel::Column::NODE)
            {
                continue;
            }

            int width(0);

            if ((*column) == NodeSelectorModel::Column::IS_EXPORTED)
            {
                // The vertical scrollbar overlays the last column; reserve its width so the
                // centered export icon is not covered by it.
                const int scrollBarWidth(ui->tMegaFolders->verticalScrollBar()->sizeHint().width());
                width = 32 + scrollBarWidth;
            }
            else if ((*column) == NodeSelectorModel::Column::LABEL)
            {
                width =
                    !mShowLabelText ?
                        compactLabelColumnWidth :
                        std::max(std::min(qRound(ui->tMegaFolders->width() * labelColumnProportion),
                                          labelColumnMaxWidth),
                                 labelColumnMinWidth);
            }
            else if ((*column) == NodeSelectorModel::Column::ADDED_DATE)
            {
                // Initial width; still user-resizable (Interactive).
                width = 130;
            }
            else if ((*column) == NodeSelectorModel::Column::LAST_MODIFIED_DATE)
            {
                // QHeaderView has no per-section maximum, so emulate "stretch up to 200px": grow
                // with the window but cap at 200; NODE (Stretch) absorbs the remaining space.
                width =
                    std::max(std::min(qRound(ui->tMegaFolders->width() * secondaryColumnProportion),
                                      maxSecondaryColumnWidth),
                             minWidth);
            }
            else
            {
                width =
                    std::max(std::min(qRound(ui->tMegaFolders->width() * secondaryColumnProportion),
                                      maxSecondaryColumnWidth),
                             minWidth);
            }

            ui->tMegaFolders->setColumnWidth((*column), width);
        }
    }
}

void NodeSelectorTreeViewWidget::rebuildVisibleColumns()
{
    // While the model is detached the header has no columns; keep the last known visible
    // set instead of clearing it to empty.
    if (!isTreeViewReady())
    {
        return;
    }

    mVisibleColumns.clear();
    for (int column = 0; column < ui->tMegaFolders->header()->count(); ++column)
    {
        if (!ui->tMegaFolders->header()->isSectionHidden(column))
        {
            mVisibleColumns.append(column);
        }
    }
}

void NodeSelectorTreeViewWidget::checkViewOnModelChange()
{
    // While the UI is blocked (a concurrent proxy job is running / the loading scene is
    // shown) the refresh is pointless and unsafe: onUiBlocked(false) runs it
    // unconditionally on unblock, so just skip.
    if (mUiBlocked)
    {
        return;
    }

    if (!mCheckViewOnModelChangeDebounce.isActive())
    {
        mCheckViewOnModelChangeDebounce.start();
    }
}

void NodeSelectorTreeViewWidget::executeCheckViewOnModelChange()
{
    setCurrentViewWidget();
    emit viewStateChanged();
}

void NodeSelectorTreeViewWidget::onModelRowsChanged()
{
    // rowsInserted/rowsRemoved fire on every change (e.g. expanding a subfolder loads its
    // children under a child parent). Only refresh when the current root itself toggles
    // between empty and non-empty; otherwise the breadcrumb/header flickers on expansion.
    const bool nowEmpty = (mProxyModel->rowCount(getCurrentRootIndex()) == 0);
    if (nowEmpty != mRootWasEmpty)
    {
        mRootWasEmpty = nowEmpty;
        checkViewOnModelChange();
    }
}

void NodeSelectorTreeViewWidget::setNewFolderInfo(const NewFolderInfo& newNewFolderInfo)
{
    mSelectionCoordinator->setNewFolderInfo(newNewFolderInfo);
}

void NodeSelectorTreeViewWidget::expandNodeByHandle(mega::MegaHandle handle)
{
    // Expanding triggers the children fetch, so the parent becomes initialised in the model.
    // A freshly created child then arrives via the visible add path (and checkNewFolderAdded
    // selects it) instead of as EXISTS_BUT_PARENT_UNINITIALISED.
    const auto index = mProxyModel->getIndexFromHandle(handle);
    if (index.isValid())
    {
        ui->tMegaFolders->setExpanded(index, true);
    }
}

void NodeSelectorTreeViewWidget::onLevelLoaded()
{
    // Initialise the view only the first time, but always refresh the empty/nav state below.
    // Use an explicit flag, not model()==nullptr: the loading scene detaches the model
    // temporarily, which would otherwise re-run the whole init (duplicate connects, etc.).
    if (!mViewInitialized)
    {
        ui->tMegaFolders->setContextMenuPolicy(Qt::DefaultContextMenu);
        ui->tMegaFolders->setExpandsOnDoubleClick(false);
        ui->tMegaFolders->header()->setDefaultAlignment(Qt::AlignLeft);
        ui->tMegaFolders->header()->setDefaultSectionSize(35);
        ui->tMegaFolders->header()->setFixedHeight(40);
        ui->tMegaFolders->setItemDelegate(createItemDelegate(ui->tMegaFolders));
        ui->tMegaFolders->setItemDelegateForColumn(NodeSelectorModel::Column::LABEL,
                                                   createLabelDelegate(ui->tMegaFolders));
        ui->tMegaFolders->setTextElideMode(Qt::ElideMiddle);

        ui->tMegaFolders->sortByColumn(NodeSelectorModel::Column::NODE, Qt::AscendingOrder);
        ui->tMegaFolders->setModel(mProxyModel.get());
        updateColumnResizeModes();
        setNonInteractiveColumns(mSelectType->nonInteractiveColumns());

        ui->tMegaFolders->header()->setVisible(true);
        ui->tMegaFolders->header()->setProperty("HeaderIconCenter", true);

        // those connects needs to be done after the model is set, do not move them
        connect(ui->tMegaFolders->selectionModel(),
                &QItemSelectionModel::selectionChanged,
                this,
                &NodeSelectorTreeViewWidget::onSelectionChanged);
        // When a model reset begins (e.g. a search) the items backing the model are
        // destroyed, but QItemSelectionModel only clears its ranges on endResetModel.
        // In that window, reading the selection (selectedRows) dereferences already
        // freed items -> crash. Clear the selection at the START of the reset, while
        // the items are still valid.
        connect(mProxyModel.get(),
                &QAbstractItemModel::modelAboutToBeReset,
                this,
                [this]()
                {
                    if (auto selModel = ui->tMegaFolders->selectionModel())
                    {
                        selModel->clear();
                    }
                });
        connect(ui->tMegaFolders,
                &NodeSelectorTreeView::deleteNodeClicked,
                this,
                &NodeSelectorTreeViewWidget::onDeleteClicked);
        connect(ui->tMegaFolders,
                &NodeSelectorTreeView::leaveShareClicked,
                this,
                &NodeSelectorTreeViewWidget::onLeaveShareClicked);
        connect(ui->tMegaFolders,
                &NodeSelectorTreeView::renameNodeClicked,
                this,
                &NodeSelectorTreeViewWidget::onRenameClicked);
        connect(ui->tMegaFolders,
                &NodeSelectorTreeView::newFolderClicked,
                this,
                [this]()
                {
                    emit onCustomButtonClicked(SelectType::ButtonId::NEW_FOLDER);
                });
        connect(ui->tMegaFolders,
                &NodeSelectorTreeView::uploadClicked,
                this,
                [this]()
                {
                    emit onCustomButtonClicked(SelectType::ButtonId::UPLOAD);
                });
        connect(ui->tMegaFolders,
                &NodeSelectorTreeView::getMegaLinkClicked,
                this,
                &NodeSelectorTreeViewWidget::onGenMEGALinkClicked);
        connect(ui->tMegaFolders,
                &QTreeView::doubleClicked,
                this,
                &NodeSelectorTreeViewWidget::onItemDoubleClick);
        connect(ui->tMegaFolders,
                &NodeSelectorTreeView::enterKeyPressed,
                this,
                &NodeSelectorTreeViewWidget::enterKeyPressed);
        connect(ui->tMegaFolders->header(),
                &QHeaderView::sectionResized,
                this,
                &NodeSelectorTreeViewWidget::onSectionResized);

        makeViewConnections();

        // The guard inside setRootIndex coerces an invalid index to the top root when one exists.
        setRootIndex(QModelIndex());

        setStyleSheet(styleSheet());

        // View ready to work with it > View init and model loaded
        mViewInitialized = true;
    }

    emit viewReady(this);
}

void NodeSelectorTreeViewWidget::onRemovedIndexAffectsCurrentRoot(const QModelIndex& indexToRemove)
{
    if (!indexToRemove.isValid())
    {
        return;
    }

    // Use the model's committed root (mapped to the proxy), not ui->tMegaFolders->rootIndex():
    // while a move/delete is in progress the loading scene detaches the model from the view
    // (setModel(nullptr)), so the view's rootIndex() is transiently invalid and this handler
    // would otherwise bail, leaving the current root dangling when the folder you are inside is
    // moved to rubbish. getCurrentRootIndex() is loading-scene independent.
    const auto currentRoot = getCurrentRootIndex();
    if (!currentRoot.isValid())
    {
        return;
    }

    bool removedIndexIsInCurrentPath = false;
    for (QModelIndex index = currentRoot; index.isValid(); index = index.parent())
    {
        if (index == indexToRemove)
        {
            removedIndexIsInCurrentPath = true;
            break;
        }
    }

    if (!removedIndexIsInCurrentPath)
    {
        return;
    }

    const auto parentIndex = indexToRemove.parent();
    // An invalid parent (top-level folder) is coerced to the top root by setRootIndex's guard.
    setRootIndex(parentIndex);
}

MegaHandle NodeSelectorTreeViewWidget::getHandleByIndex(const QModelIndex& idx) const
{
    return mProxyModel ? mProxyModel->getHandle(idx) : mega::INVALID_HANDLE;
}

void NodeSelectorTreeViewWidget::addHandleToBeReplaced(mega::MegaHandle handle)
{
    mNodesToBeReplaced.insert(handle);
}

QModelIndex NodeSelectorTreeViewWidget::getIndexFromHandle(const mega::MegaHandle& handle)
{
    return mProxyModel ? mProxyModel->getIndexFromHandle(handle) : QModelIndex();
}

QModelIndex NodeSelectorTreeViewWidget::getRootIndexFromIndex(const QModelIndex& index)
{
    QModelIndex parentIndex(index);
    while (parentIndex.parent().isValid())
    {
        parentIndex = parentIndex.parent();
    }
    return parentIndex;
}

bool NodeSelectorTreeViewWidget::isAllowedToEnterInIndex(const QModelIndex& idx)
{
    return mSelectType->isAllowedToNavigateInside(idx);
}

bool NodeSelectorTreeViewWidget::isDownloadAllowed() const
{
    return mSelectType->isDownloadAllowed();
}

void NodeSelectorTreeViewWidget::onItemDoubleClick(const QModelIndex& index)
{
    if (isDownloadAllowed())
    {
        auto item = mModel->getItemByIndex(index);
        if (!item || !item->getNode())
        {
            return;
        }

        if (item->getNode()->isFile() && !item->isTakenDown())
        {
            MegaSyncApp->downloadACtionClickedWithHandles(QList<mega::MegaHandle>()
                                                          << item->getNode()->getHandle());
            return;
        }
    }

    if (mSelectType->isFilePicker())
    {
        // File pickers don't navigate into folders: double-click only toggles the tree branch.
        ui->tMegaFolders->setExpanded(index, !ui->tMegaFolders->isExpanded(index));
    }
    else if (isAllowedToEnterInIndex(index))
    {
        setRootIndex(index);
    }
}

std::shared_ptr<NodeSelectorProxyModel> NodeSelectorTreeViewWidget::createProxyModel()
{
    return mSelectType->createProxyModel();
}

void NodeSelectorTreeViewWidget::setLoadingSceneVisible(bool blockUi)
{
    if (blockUi && ui->stackedWidget->currentWidget() != ui->treeViewPage)
    {
        setCurrentPage(ViewType::VIEW);
    }

    ui->tMegaFolders->loadingView().toggleLoadingScene(blockUi);
}

bool NodeSelectorTreeViewWidget::isLoading() const
{
    return ui->tMegaFolders->loadingView().isLoadingViewSet();
}

void NodeSelectorTreeViewWidget::selectPendingIndexes()
{
    mSelectionCoordinator->selectPendingIndexes();
}

void NodeSelectorTreeViewWidget::setViewPage()
{
    if (mModel)
    {
        auto topRootIndex = mModel->hasTopRootIndex() ? mModel->index(0, 0) : QModelIndex();

        if (mModel->rowCount(topRootIndex) == 0 && showEmptyView())
        {
            setCurrentPage(ViewType::ROOT_EMPTY);
            return;
        }

        setCurrentPage(ViewType::VIEW);
    }
}

void NodeSelectorTreeViewWidget::updateEmptyStateButtonsVisibility()
{
    if (!mSelectType || ui->stackedWidget->currentWidget() != ui->emptyPage)
    {
        return;
    }

    if (mCurrentViewType == ViewType::FOLDER_EMPTY)
    {
        setEmptyStateButtonsVisibility(getEmptyFolderPageInfo());
    }
    else if (mCurrentViewType == ViewType::ROOT_EMPTY)
    {
        setEmptyStateButtonsVisibility(getEmptyRootPageInfo());
    }
}

QModelIndex NodeSelectorTreeViewWidget::indexForBreadcrumbSegment(int segmentIndex) const
{
    if (!mProxyModel || segmentIndex < 0)
    {
        return QModelIndex();
    }

    QList<QModelIndex> path;
    for (QModelIndex index = getCurrentRootIndex(); index.isValid(); index = index.parent())
    {
        path.prepend(index);
    }

    if (!mProxyModel->getTopRootIndex().isValid())
    {
        if (segmentIndex == 0)
        {
            return QModelIndex();
        }

        --segmentIndex;
    }

    return segmentIndex < path.size() ? path.at(segmentIndex) : QModelIndex();
}

QModelIndex NodeSelectorTreeViewWidget::getAddedNodeParent(mega::MegaHandle parentHandle)
{
    return mModel->findIndexByNodeHandle(parentHandle, QModelIndex());
}

void NodeSelectorTreeViewWidget::onUiBlocked(bool state)
{
    // Blocking always precedes the concurrent proxy job (sort() emits blockUi(true)
    // synchronously before launching it), so cancelling here guarantees the debounced
    // refresh can never touch the proxy mid-job; the unblock branch below re-runs the
    // same refresh (setCurrentViewWidget + viewStateChanged) unconditionally.
    mCheckViewOnModelChangeDebounce.stop();

    if (mUiBlocked != state)
    {
        mUiBlocked = state;
    }

    // Hide the header separator while the loading scene is shown.
    ui->headerDivider->setVisible(!state);

    emit uiIsBlocked(mUiBlocked);
    ui->searchButtonsWidget->setDisabled(state);

    if (!state)
    {
        setCurrentViewWidget();
        onSelectionHasChanged();
        mSelectionCoordinator->expandPendingIndexes();
        mSelectionCoordinator->selectPendingIndexes();
        // The model is reattached now (the loading scene hid), so the header has its columns back.
        // Column visibility is no longer buffered by the loading scene; re-apply it here (the owner
        // reconfigures on modelReattached) BEFORE recomputing widths so hidden columns get none.
        emit modelReattached(this);
        // QHeaderView::restoreState() (run by the loading scene on reattach) does NOT restore
        // per-section resize modes, so NODE loses its Stretch on every reattach; re-assert the
        // modes before recomputing the widths that were skipped while detached.
        updateColumnResizeModes();
        updateColumnsWidth(true);

        // Button visibility was computed while the model was detached (the read-only
        // fallback hid Upload/New folder). Now that the live model is back, re-emit so
        // the header recomputes against the real root access. viewStateChanged() drives
        // refreshHeader(), which already recomputes button visibility.
        emit viewStateChanged();
    }
}

void NodeSelectorTreeViewWidget::onSelectionChanged(const QItemSelection& selected,
                                                    const QItemSelection& deselected)
{
    Q_UNUSED(deselected)

    if (!mUiBlocked)
    {
        // A non-empty user selection (our own re-selection is silenced, the post-select
        // notify passes an empty selection) means the user took over: stop re-applying
        // the moved node's selection.
        if (!selected.isEmpty())
        {
            mSelectionCoordinator->clearMovedSelection();
        }

        onSelectionHasChanged();
    }
}

void NodeSelectorTreeViewWidget::onModelModified()
{
    emit modelModified();

    const bool nowEmpty = (mProxyModel->rowCount(getCurrentRootIndex()) == 0);
    if (nowEmpty != mWasEmpty)
    {
        mWasEmpty = nowEmpty;
        setCurrentViewWidget();
        emit viewStateChanged();
        emit viewButtonsStateChanged();
    }
}

void NodeSelectorTreeViewWidget::onSelectionHasChanged()
{
    emit selectionHasChanged();
}

void NodeSelectorTreeViewWidget::onRenameClicked()
{
    mNodeActions.renameNode(getSelectedNodeHandle());
}

void NodeSelectorTreeViewWidget::onDeleteClicked(const QList<mega::MegaHandle>& handles,
                                                 bool permanently,
                                                 bool showConfirmationMessageBox)
{
    mNodeActions.deleteNodes(handles, permanently, showConfirmationMessageBox);
}

void NodeSelectorTreeViewWidget::onLeaveShareClicked(const QList<mega::MegaHandle>& handles)
{
    mNodeActions.leaveShare(handles);
}

NodeSelectorTreeViewWidget::NodeState
    NodeSelectorTreeViewWidget::getNodeOnModelState(const QModelIndex& index, mega::MegaNode* node)
{
    NodeState result(NodeState::DOESNT_EXIST);

    auto parentIndex = mModel->findIndexByNodeHandle(node->getParentHandle(), QModelIndex());

    if (parentIndex.isValid())
    {
        auto parentItem = mModel->getItemByIndex(parentIndex);
        if (parentItem->areChildrenInitialized())
        {
            if (index.parent() == parentIndex)
            {
                result = NodeState::EXISTS;
            }
            else if (index.isValid())
            {
                result = NodeState::MOVED;
            }
            else
            {
                result = NodeState::ADD;
            }
        }
        else
        {
            if (index.isValid())
            {
                result = NodeState::MOVED_OUT_OF_VIEW;
            }
            else
            {
                result = NodeState::EXISTS_BUT_PARENT_UNINITIALISED;
            }
        }
    }
    else if (index.isValid())
    {
        result = NodeState::REMOVE;
    }
    else if (isNodeCompatibleWithModel(node))
    {
        result = NodeState::EXISTS_BUT_OUT_OF_VIEW;
    }

    return result;
}

bool NodeSelectorTreeViewWidget::onNodesUpdate(mega::MegaApi*, mega::MegaNodeList* nodes)
{
    if (!mModelUpdateCoordinator)
    {
        return false;
    }

    if (!mModelUpdateCoordinator->onNodesUpdate(nodes))
    {
        return false;
    }

    if (mModelUpdateCoordinator->shouldUpdateImmediately(IMMEDIATE_CHECK_UPDATES_NODES_THRESHOLD))
    {
        if (mNodesUpdateTimer.interval() != 0)
        {
            mNodesUpdateTimer.setInterval(0);
        }
    }
    else if (mNodesUpdateTimer.interval() != CHECK_UPDATED_NODES_INTERVAL)
    {
        mNodesUpdateTimer.setInterval(CHECK_UPDATED_NODES_INTERVAL);
    }

    return true;
}

void NodeSelectorTreeViewWidget::selectIndex(const mega::MegaHandle& handle,
                                             bool setCurrent,
                                             bool exclusiveSelect)
{
    auto index(mProxyModel->getIndexFromHandle(handle));
    if (index.isValid())
    {
        selectIndex(index, setCurrent, exclusiveSelect);
    }
}

void NodeSelectorTreeViewWidget::selectIndex(const QModelIndex& index,
                                             bool setCurrent,
                                             bool exclusiveSelect)
{
    auto selectionFlag(exclusiveSelect ? QItemSelectionModel::ClearAndSelect :
                                         QItemSelectionModel::Select);

    auto selectionModel = ui->tMegaFolders->selectionModel();
    if (selectionModel != nullptr)
    {
        if (setCurrent)
        {
            selectionModel->setCurrentIndex(index, selectionFlag | QItemSelectionModel::Rows);
        }

        selectionModel->select(index, selectionFlag | QItemSelectionModel::Rows);
    }
    else
    {
        mega::MegaApi::log(
            mega::MegaApi::LOG_LEVEL_ERROR,
            QString::fromUtf8("Invalid selectionModel access.").toUtf8().constData());
    }

    ui->tMegaFolders->scrollTo(index, QAbstractItemView::ScrollHint::PositionAtCenter);

    // Keep this scroll: when this runs during the loading-scene reattach, applyLoadingViewScroll()
    // would otherwise restore the pre-load scroll position and push a deep target out of view.
    ui->tMegaFolders->markScrollHandledExternally();
}

bool NodeSelectorTreeViewWidget::increaseMovingNodes(int number)
{
    mSelectionCoordinator->resetMoveNodesToSelect();
    return mModel->increaseMovingNodes(number);
}

bool NodeSelectorTreeViewWidget::decreaseMovingNodes(int number)
{
    return mModel->moveProcessedByNumber(number);
}

void NodeSelectorTreeViewWidget::finishMovingNodes()
{
    mModel->finishMovingNodes();
}

bool NodeSelectorTreeViewWidget::areItemsAboutToBeMovedFromHere(mega::MegaHandle firstHandleMoved)
{
    auto itemIndex(mModel->findIndexByNodeHandle(firstHandleMoved, QModelIndex()));
    if (itemIndex.isValid())
    {
        return true;
    }

    return false;
}

void NodeSelectorTreeViewWidget::processCachedNodesUpdated()
{
    if (mModelUpdateCoordinator)
    {
        mModelUpdateCoordinator->processCachedNodesUpdated();
    }
}

void NodeSelectorTreeViewWidget::setParentOfRestoredNodes(
    const QSet<mega::MegaHandle>& parentOfRestoredNodes)
{
    mSelectionCoordinator->setParentOfRestoredNodes(parentOfRestoredNodes);
}

void NodeSelectorTreeViewWidget::setMergeFolderHandles(
    const QMultiHash<SourceHandle, TargetHandle>& handles)
{
    mSelectionCoordinator->setMergeFolderHandles(handles);
}

void NodeSelectorTreeViewWidget::resetMergeFolderHandles(
    const QMultiHash<SourceHandle, TargetHandle>& handles)
{
    mSelectionCoordinator->resetMergeFolderHandles(handles);
}

bool NodeSelectorTreeViewWidget::isUiBlocked()
{
    return mUiBlocked;
}

void NodeSelectorTreeViewWidget::dropIntoRootIndex(QDropEvent* event)
{
    // On the empty page the drop target is the current folder, not the top root.
    auto target = mModel->getCurrentRootIndex();
    if (!event->mimeData()->urls().isEmpty() || mModel->canDropMimeData(event->mimeData(),
                                                                        Qt::MoveAction,
                                                                        target.row(),
                                                                        target.column(),
                                                                        target.parent()))
    {
        ui->tMegaFolders->dropEvent(event);
    }
}

void NodeSelectorTreeViewWidget::setSelectedNodeHandle(const MegaHandle& selectedHandle)
{
    mSelectionCoordinator->setSelectedNodeHandle(selectedHandle);
}

QList<MegaHandle> NodeSelectorTreeViewWidget::getMultiSelectionNodeHandle()
{
    // Use the same selection source as okButtonEnabled() (getSelectedIndexes(), which
    // falls back to the current folder when nothing is explicitly selected): Ok enablement
    // and the handles actually returned must never disagree, otherwise Ok can accept with
    // an empty list and silently do nothing.
    return ui->tMegaFolders->getMultiSelectionNodeHandle(getSelectedIndexes());
}

QModelIndexList NodeSelectorTreeViewWidget::getSelectedIndexes() const
{
    QModelIndexList selectedIndexes{ui->tMegaFolders->selectedRows()};

    if (selectedIndexes.isEmpty() && !isCurrentRootIndexReadOnly() &&
        mSelectType->isCurrentFolderValidForSelection())
    {
        auto rootIndex = getCurrentRootIndex();
        if (rootIndex.isValid())
        {
            selectedIndexes.append(rootIndex);
        }
    }

    return selectedIndexes;
}

MegaHandle NodeSelectorTreeViewWidget::getSelectedNodeHandle() const
{
    auto selectedCurrentIndex{getSelectedIndexes()};
    return !selectedCurrentIndex.isEmpty() ? getHandleByIndex(selectedCurrentIndex.first()) :
                                             mega::INVALID_HANDLE;
}

bool NodeSelectorTreeViewWidget::containsTakenDownSelected() const
{
    return ui->tMegaFolders->containsTakenDownItem(ui->tMegaFolders->selectedRows());
}

void NodeSelectorTreeViewWidget::notifyViewStateChanged()
{
    emit viewStateChanged();
}

void NodeSelectorTreeViewWidget::notifyButtonsStateChanged()
{
    emit viewButtonsStateChanged();
}

void NodeSelectorTreeViewWidget::setRootIndex(const QModelIndex& proxy_idx)
{
    // Guard: never root the view at an invalid index while a valid top root exists; coerce to it
    // so the view and the model stay consistent (mirrors NodeSelectorModel::commitCurrentRootIndex,
    // which applies the same coercion on the model side).
    const QModelIndex effectiveProxyIdx = (!proxy_idx.isValid() && mModel->hasTopRootIndex()) ?
                                              mProxyModel->getTopRootIndex() :
                                              proxy_idx;

    QModelIndex node_column_idx;

    // In case the idx is coming from a potentially hidden column, we always take the NODE column
    // As it is the only one that have childrens
    if (effectiveProxyIdx.isValid() && effectiveProxyIdx.model() == mProxyModel.get())
    {
        node_column_idx =
            effectiveProxyIdx.sibling(effectiveProxyIdx.row(), NodeSelectorModel::Column::NODE);
    }

    auto modelRootIndex(mProxyModel->mapToSource(node_column_idx));

    if (mModel->getCurrentRootIndex() == modelRootIndex)
    {
        return;
    }

    mModel->setCurrentRootIndex(modelRootIndex);
    ui->tMegaFolders->setRootIndex(node_column_idx);
    ui->tMegaFolders->setRootIndexReadOnly(isCurrentRootIndexReadOnly());
    if (auto selectionModel = ui->tMegaFolders->selectionModel())
    {
        selectionModel->clearSelection();

        auto currentIndex = ui->tMegaFolders->rootIndex();
        if (currentIndex.isValid() && currentIndex.model() == mProxyModel.get() &&
            mProxyModel->rowCount(currentIndex) > 0)
        {
            currentIndex = mProxyModel->index(0, NodeSelectorModel::Column::NODE, currentIndex);
        }

        selectionModel->setCurrentIndex(currentIndex, QItemSelectionModel::NoUpdate);
    }

    setCurrentViewWidget();
    notifyViewStateChanged();
}

void NodeSelectorTreeViewWidget::setCurrentViewWidget()
{
    if (!mProxyModel)
    {
        return;
    }

    auto currentRootIndex(getCurrentRootIndex());
    auto topRootIndex(mProxyModel->getTopRootIndex());

    // The empty-state flags (mWasEmpty / mRootWasEmpty) are owned by setCurrentPage(), which
    // sets them from the page actually shown. Here we only need the emptiness of the current
    // root to choose between the "Empty folder" page and the normal view.
    const bool isEmpty = (mProxyModel->rowCount(currentRootIndex) == 0);

    // If we are inside a folder, show the "Empty folder" page.
    if ((currentRootIndex != topRootIndex) && isEmpty)
    {
        if (ui && ui->tMegaFolders->loadingView().isLoadingViewSet())
        {
            return;
        }

        setCurrentPage(ViewType::FOLDER_EMPTY);
    }
    else
    {
        setViewPage();
    }
}

SelectType::EmptyPageInfo NodeSelectorTreeViewWidget::getEmptyRootPageInfo()
{
    return SelectType::EmptyPageInfo();
}

SelectType::EmptyPageInfo NodeSelectorTreeViewWidget::getEmptyFolderPageInfo() const
{
    switch (getTabType())
    {
        case TabItem::SHARES:
        case TabItem::BACKUPS:
        case TabItem::RUBBISH:
        {
            SelectType::EmptyPageInfo info;
            info.title = tr("Empty folder");
            info.icon.addFile(Utilities::getPixmapName(QLatin1String("glass-folder"),
                                                       Utilities::AttributeType::NONE));
            return info;
        }
        default:
        {
            return mSelectType ? mSelectType->getEmptyFolderPageInfo() :
                                 SelectType::EmptyPageInfo();
        }
    }
}

NodeSelectorDelegate* NodeSelectorTreeViewWidget::createItemDelegate(QObject* parent)
{
    return new NodeRowDelegate(parent);
}

NodeSelectorDelegate* NodeSelectorTreeViewWidget::createLabelDelegate(QObject* parent)
{
    return new NodeLabelDelegate(showLabelText(), parent);
}

void NodeSelectorTreeViewWidget::setColumnHidden(int column, bool hidden)
{
    if (ui->tMegaFolders->isColumnHidden(column) == hidden)
    {
        return;
    }
    ui->tMegaFolders->setColumnHidden(column, hidden);
    // Don't recompute widths here (the model may be detached / viewport unsettled during
    // column configuration). Just refresh the visible set; widths are recomputed on the
    // next real viewport resize and after the model is reattached.
    rebuildVisibleColumns();
}

void NodeSelectorTreeViewWidget::setNonInteractiveColumns(const QSet<int>& columns)
{
    if (auto header = qobject_cast<NodeSelectorHeaderView*>(ui->tMegaFolders->header()))
    {
        auto nonInteractiveColumns(columns);
        nonInteractiveColumns.insert(NodeSelectorModel::Column::IS_EXPORTED);
        header->setNonInteractiveSections(nonInteractiveColumns);
    }
}

void NodeSelectorTreeViewWidget::setInitialShowLabelText(bool show)
{
    if (ui->tMegaFolders->model())
    {
        Q_ASSERT_X(false,
                   "NodeSelectorTreeViewWidget::setInitialShowLabelText",
                   "Label text visibility must be configured before init().");
        return;
    }

    if (mShowLabelText == show)
    {
        return;
    }

    mShowLabelText = show;
}

bool NodeSelectorTreeViewWidget::showLabelText() const
{
    return mShowLabelText;
}

void NodeSelectorTreeViewWidget::resetAutoColumnWidths()
{
    mManuallyResizedColumn = false;
    mResizeEventsReceived = 0;
    mResizeEventsTimer.stop();
    updateColumnsWidth(true);
}

bool NodeSelectorTreeViewWidget::isTreeViewReady() const
{
    return ui && ui->tMegaFolders && ui->tMegaFolders->model() &&
           ui->tMegaFolders->header()->count() > 0;
}

void NodeSelectorTreeViewWidget::updateColumnResizeModes()
{
    if (!isTreeViewReady())
    {
        return;
    }

    auto* header = ui->tMegaFolders->header();

    // NODE is the only elastic column: it stretches to fill the remaining space, so resizing
    // any other column steals space from NODE alone. Disable stretchLastSection (QTreeView
    // enables it by default), otherwise the last column would also stretch and fight NODE.
    header->setStretchLastSection(false);
    header->setSectionResizeMode(NodeSelectorModel::Column::NODE, QHeaderView::Stretch);

    header->setSectionResizeMode(NodeSelectorModel::Column::LABEL,
                                 mShowLabelText ? QHeaderView::Interactive : QHeaderView::Fixed);

    if (!mSelectType->isFilePicker())
    {
        header->setSectionResizeMode(NodeSelectorModel::Column::ADDED_DATE,
                                     QHeaderView::Interactive);
        header->setSectionResizeMode(NodeSelectorModel::Column::LAST_MODIFIED_DATE,
                                     QHeaderView::Interactive);
    }

    header->setSectionResizeMode(NodeSelectorModel::Column::IS_EXPORTED, QHeaderView::Fixed);
}

QModelIndex NodeSelectorTreeViewWidget::getParentIncomingShareByIndex(QModelIndex idx) const
{
    while (idx.isValid())
    {
        if (auto item = NodeSelectorModel::getItemByIndex(idx))
        {
            if (item->getNode()->isInShare())
            {
                return idx;
            }
            else
            {
                idx = idx.parent();
            }
        }
    }
    return QModelIndex();
}

void NodeSelectorTreeViewWidget::onGenMEGALinkClicked(const QList<mega::MegaHandle>& handles)
{
    mNodeActions.exportLinks(handles);
}
