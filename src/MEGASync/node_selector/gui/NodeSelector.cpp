#include "NodeSelector.h"

#include "DestinationBreadcrumb.h"
#include "DialogOpener.h"
#include "DuplicatedNodeDialog.h"
#include "IncomingShareHeaderWidget.h"
#include "megaapi.h"
#include "MegaApplication.h"
#include "MegaNodeNames.h"
#include "MessageDialogOpener.h"
#include "NewFolderDialog.h"
#include "NodeSelectorModel.h"
#include "NodeSelectorProxyModel.h"
#include "NodeSelectorTreeViewWidgetSpecializations.h"
#include "SearchLineEdit.h"
#include "TabSelector.h"
#include "ui_NodeSelector.h"
#include "Utilities.h"
#include "ViewLoadingScene.h"

#include <QKeyEvent>
#include <QLayout>
#include <QMessageBox>
#include <QPointer>
#include <QShortcut>

#include <optional>

using namespace mega;

NodeSelector::NodeSelector(SelectTypeSPtr selectType, QWidget* parent):
    QDialog(parent),
    mMegaApi(MegaSyncApp->getMegaApi()),
    ui(new Ui::NodeSelector),
    mSelectType(selectType),
    mDelegateListener(std::make_unique<QTMegaListener>(mMegaApi, this)),
    mInitialised(false),
    mDuplicatedType(std::nullopt),
    mDuplicatedModel(nullptr),
    mTargetWid(nullptr)
{
    ui->setupUi(this);
    ui->destinationBreadcrumb->showDefaultUploadOption(false);

    connect(ui->stackedWidget,
            &QStackedWidget::currentChanged,
            this,
            &NodeSelector::onCurrentTreeViewWidgetChanged);

    mMegaApi->addListener(mDelegateListener.get());

    connect(ui->fIncomingShares,
            &TabSelector::clicked,
            this,
            &NodeSelector::onbShowIncomingSharesClicked,
            Qt::QueuedConnection);
    connect(ui->fCloudDrive, &TabSelector::clicked, this, &NodeSelector::onbShowCloudDriveClicked);
    connect(ui->fBackups, &TabSelector::clicked, this, &NodeSelector::onbShowBackupsFolderClicked);
    connect(ui->fRubbish, &TabSelector::clicked, this, &NodeSelector::onbShowRubbishClicked);

    ui->fCloudDrive->connectToDropEvent(
        [this](std::shared_ptr<QDropEvent> event)
        {
            onCloudDriveTabDropped(event);
        });

    updateNodeSelectorTabs();
    onOptionSelected(NodeSelectorTreeViewWidget::TabItem::CLOUD_DRIVE);

    resetButtonsText();

    ui->bOk->setDefault(true);
    ui->bOk->setEnabled(false);

    connect(ui->bOk, &QPushButton::clicked, this, &NodeSelector::confirmSelection);
    connect(ui->bCancel, &QPushButton::clicked, this, &NodeSelector::reject);

    connect(ui->leSearchTool, &SearchLineEdit::search, this, &NodeSelector::onSearch);

    ui->incomingShareContainer->hide();

    connect(ui->incomingShareHeaderWidget,
            &IncomingShareHeaderWidget::menuRequested,
            this,
            [this](const QPoint& globalPos)
            {
                if (auto* wid = getCurrentTreeViewWidget())
                {
                    wid->showCurrentRootContextMenu(globalPos);
                }
            });

    resize(1024, 720);
    setMinimumSize(660, 560);
}

NodeSelector::~NodeSelector()
{
    // Remove duplicated node dialog if it is currently open
    if (mDuplicatedType.has_value())
    {
        DialogOpener::closeDialogsByClass<DuplicatedNodeDialog>();
    }
    mMegaApi->removeListener(mDelegateListener.get());
    delete ui;
}

void NodeSelector::init()
{
    configureSidebar();
    configureHeader();
    configureActionButtonsPlacement();
    configureFooter();

    createActionButtons();
    createSpecialisedTreeViewWidgets();
    addSearchTreeViewWidget();
    specialisedTreeViewWidgetsCreated();
    onCurrentTreeViewWidgetChanged(ui->stackedWidget->currentIndex());

    connect(ui->leSearchTool, &SearchLineEdit::cleared, this, &NodeSelector::hideGhostSearch);

    ui->leSearchTool->setContainerStyle(QLatin1String("page-background"),
                                        QLatin1String("border-strong"),
                                        8);

    mInitialised = true;
}

void NodeSelector::changeEvent(QEvent* event)
{
    if (event && event->type() == QEvent::WindowStateChange && !isMaximized())
    {
        for (int index = 0; index < ui->stackedWidget->count(); ++index)
        {
            if (auto wid =
                    qobject_cast<NodeSelectorTreeViewWidget*>(ui->stackedWidget->widget(index)))
            {
                wid->resetAutoColumnWidths();
            }
        }
    }

    QDialog::changeEvent(event);
}

void NodeSelector::updateNodeSelectorTabs()
{
    auto updateTabTitle = [](TabSelector* tab, const QString& title)
    {
        tab->setTitle(title);
        tab->setAccessibleName(title);
    };

    updateTabTitle(ui->fCloudDrive, MegaNodeNames::getCloudDriveName());
    updateTabTitle(ui->fIncomingShares, MegaNodeNames::getIncomingSharesName());
    updateTabTitle(ui->fBackups, MegaNodeNames::getBackupsName());
    updateTabTitle(ui->fRubbish, MegaNodeNames::getRubbishName());
}

void NodeSelector::onSearch(const QString& text)
{
    handleSearch(text);
}

void NodeSelector::onbNewFolderClicked()
{
    auto sourceWidget = getCurrentTreeViewWidget();

    if (!sourceWidget)
    {
        return;
    }

    auto parentNode = getNewFolderParentNode(sourceWidget);
    if (!parentNode)
    {
        parentNode = MegaSyncApp->getRootNode();
        if (!parentNode)
        {
            return;
        }
    }

    QPointer<NewFolderDialog> dialog(new NewFolderDialog(parentNode, this));
    dialog->init();
    // Initialise the parent's children in the model the moment creation is confirmed, so the new
    // node arrives through the visible add path (addNodes -> nodesAdded -> checkNewFolderAdded)
    // instead of as EXISTS_BUT_PARENT_UNINITIALISED.
    connect(dialog,
            &NewFolderDialog::creatingFolder,
            sourceWidget,
            &NodeSelectorTreeViewWidget::expandNodeByHandle);
    DialogOpener::showDialog(
        dialog,
        [this, dialog, sourceWidget]()
        {
            auto newNode = dialog->getNewNode();
            // IF the dialog return a node, there are two scenarios:
            // 1) The dialog has been accepted, a new folder has been created
            // 2) The dialog has been rejected because the folder already
            // exists. If so, select the existing folder
            if (newNode)
            {
                applyNewFolderSelection(sourceWidget, newNode.get());

                // Focusing a widget whose top-level window is not the active
                // window makes Qt call QWindow::requestActivate()
                // (QWidgetPrivate::setFocus_sys). Wayland does not support it
                // and older Qt 5.15 builds crash there, so only force the
                // activation/focus when our window is already active. When it
                // is not (e.g. just after the NewFolderDialog closes on
                // Wayland), the compositor restores focus on its own.
                const QWindow* selectorWindow = window() ? window()->windowHandle() : nullptr;
                const bool windowAlreadyActive =
                    selectorWindow && selectorWindow == QGuiApplication::focusWindow();
                if (!Platform::getInstance()->isWayland() || windowAlreadyActive)
                {
#ifdef Q_OS_LINUX
                    // It seems that the NodeSelector is not activated when the
                    // NewFolderDialog is closed, so the ui->tMegaFolders is
                    // not correctly focused
                    qApp->setActiveWindow(this);
#endif

                    // Set the focus to the view to allow the user to press
                    // enter (or go back, in a future feature)
                    setFocus();
                }
            }
        });
}

void NodeSelector::onUiIsBlocked(bool state)
{
    mUiBlocked = state;
    // A search finishes when the view unblocks.
    if (!state)
    {
        mSearchInProgress = false;
    }

    ui->leSearchTool->setDisabled(state);
    ui->header->setDisabled(state);
    ui->customButtonsContainer->setDisabled(state);
    if (state)
    {
        ui->bOk->setDisabled(true);
    }
    // bCancel and wLeftPaneNS stay enabled so the user can always abort or navigate tabs.

    // Hide the search result count while a search/load is in progress; refresh it (showing the
    // final count) once it finishes.
    refreshSearchResultCount();
}

void NodeSelector::showSearchingIndicator()
{
    ui->lSearchResultCount->setText(tr("Searching…"));
    // Render it muted (disabled look) from the start, matching how it appears once the header is
    // disabled during the search — instead of flashing the enabled style on the first search.
    ui->lSearchResultCount->setEnabled(false);
    ui->lSearchResultCount->setVisible(true);
}

void NodeSelector::onSelectionChanged()
{
    updateOkButtonState(getCurrentTreeViewWidget());
    refreshHeaderButtons(getCurrentTreeViewWidget());
    refreshDestinationBreadcrumb();
}

void NodeSelector::updateOkButtonState(NodeSelectorTreeViewWidget* wid)
{
    if (wid)
    {
        auto isSelectionCorrect = mSelectType->okButtonEnabled(wid->getSelectedIndexes());
        ui->bOk->setEnabled(isSelectionCorrect);
    }
}

QString NodeSelector::folderNameForWidget(NodeSelectorTreeViewWidget* wid) const
{
    if (!wid)
    {
        return QString();
    }

    const auto rootIndex = wid->getCurrentRootIndex();
    if (rootIndex.isValid())
    {
        return rootIndex.data(Qt::DisplayRole).toString();
    }

    return wid->getRootText();
}

void NodeSelector::applyHeaderFolderInfoState(NodeSelectorTreeViewWidget* wid)
{
    if (!wid)
    {
        return;
    }

    const auto incomingInfo = wid->incomingShareHeaderData();
    const auto showIncomingInfo = incomingInfo.has_value();

    ui->incomingShareContainer->setVisible(showIncomingInfo);

    if (incomingInfo)
    {
        ui->incomingShareHeaderWidget->setData(*incomingInfo);
    }
    else
    {
        ui->incomingShareHeaderWidget->clear();
    }
}

void NodeSelector::applyHeaderButtonsState(NodeSelectorTreeViewWidget* wid)
{
    if (!wid)
    {
        return;
    }

    mSelectType->checkActionButtonsVisibility(wid, mButtons);
}

void NodeSelector::applySearchToolVisibilityState(NodeSelectorTreeViewWidget* wid,
                                                  NodeSelectorTreeViewWidget::ViewType type)
{
    const bool shouldShowSearchTool =
        wid && !wid->isLoading() &&
        (wid == mSearchWidget || type == NodeSelectorTreeViewWidget::ViewType::VIEW);
    ui->leSearchTool->setVisible(shouldShowSearchTool);
}

void NodeSelector::updateHeaderTopRowVisibility()
{
    // isVisibleTo ignores the container's own state, so this still works when it was
    // previously hidden.
    const auto hasVisibleChild = [](QWidget* container)
    {
        auto* layout = container ? container->layout() : nullptr;
        for (int i = 0; layout && i < layout->count(); ++i)
        {
            auto* child = layout->itemAt(i)->widget();
            if (child && child->isVisibleTo(container))
            {
                return true;
            }
        }
        return false;
    };

    // Collapse the empty action buttons container so the row can hide when nothing is left.
    ui->actionButtonsContainer->setVisible(hasVisibleChild(ui->actionButtonsContainer));
    ui->searchRow->setVisible(hasVisibleChild(ui->searchRow));
}

void NodeSelector::refreshHeaderButtons(NodeSelectorTreeViewWidget* wid)
{
    if (!wid || wid != getCurrentTreeViewWidget())
    {
        return;
    }

    applyHeaderButtonsState(wid);
}

void NodeSelector::refreshHeader(NodeSelectorTreeViewWidget* wid)
{
    if (!wid || wid != getCurrentTreeViewWidget())
    {
        return;
    }

    applyHeaderButtonsState(wid);
    applyHeaderFolderInfoState(wid);
    applySearchToolVisibilityState(wid, wid->currentViewPage());
    refreshBreadcrumbs();
    updateHeaderTopRowVisibility();
}

void NodeSelector::refreshBreadcrumbs()
{
    refreshDestinationBreadcrumb();
    refreshSearchResultCount();
}

void NodeSelector::addCurrentWidgetConnection(const QMetaObject::Connection& connection)
{
    mCurrentWidgetConnections.append(connection);
}

void NodeSelector::disconnectCurrentWidgetConnections()
{
    for (const auto& connection: mCurrentWidgetConnections)
    {
        disconnect(connection);
    }
    mCurrentWidgetConnections.clear();
}

void NodeSelector::showDefaultUploadOption(bool show)
{
    ui->destinationBreadcrumb->showDefaultUploadOption(show);
}

void NodeSelector::setDefaultUploadOption(bool value)
{
    ui->destinationBreadcrumb->setDefaultUploadOption(value);
}

bool NodeSelector::getDefaultUploadOption()
{
    return ui->destinationBreadcrumb->getDefaultUploadOption();
}

bool NodeSelector::event(QEvent* event)
{
    if (event->type() == QEvent::LanguageChange)
    {
        ui->retranslateUi(this);
        resetButtonsText();
        updateNodeSelectorTabs();
        onLanguageChangeEvent();
        if (auto widg = getCurrentTreeViewWidget())
        {
            if (mSelectType)
            {
                mSelectType->updateActionButtonsText(mButtons);
            }
            refreshHeader(widg);
        }
        else
        {
            refreshBreadcrumbs();
        }

        refreshNavigationBreadcrumb();
    }

    return QDialog::event(event);
}

void NodeSelector::mousePressEvent(QMouseEvent* event)
{
    if (shouldClearSelectionOnBackgroundClick(event->pos()))
    {
        for (int page = 0; page < ui->stackedWidget->count(); ++page)
        {
            auto viewContainer = getTreeViewWidget(page);
            if (viewContainer)
            {
                viewContainer->clearSelection();
            }
        }
    }

    QDialog::mousePressEvent(event);
}

void NodeSelector::confirmSelection()
{
    auto currentViewContainer = getCurrentTreeViewWidget();
    for (int page = 0; page < ui->stackedWidget->count(); ++page)
    {
        auto viewContainer = getTreeViewWidget(page);
        if (viewContainer && viewContainer != currentViewContainer)
        {
            viewContainer->abort();
        }
    }

    onOkButtonClicked();
}

void NodeSelector::performItemsToBeMoved(const QList<mega::MegaHandle>& handles,
                                         IncreaseOrDecrease type,
                                         bool blockSource,
                                         bool blockTarget)
{
    if (handles.isEmpty())
    {
        return;
    }

    mTargetWid = nullptr;
    mSourceWids.clear();

    // IF we want to block the source or target, set values to false in order to look for them
    bool foundSource(!blockSource);
    bool foundTarget(!blockTarget);

    auto senderModel(dynamic_cast<NodeSelectorModel*>(sender()));

    auto targetOrSourceFound = [type](NodeSelectorTreeViewWidget* wid, int nodesUpdateToReceive)
    {
        if (wid)
        {
            if (type == IncreaseOrDecrease::INCREASE)
            {
                wid->increaseMovingNodes(nodesUpdateToReceive);
            }
            else
            {
                wid->decreaseMovingNodes(nodesUpdateToReceive);
            }
        }
    };

    auto findSource = [this, handles](NodeSelectorTreeViewWidget* wid)
    {
        if (wid->areItemsAboutToBeMovedFromHere(handles.first()))
        {
            mSourceWids.append(wid);
        }
    };

    for (int index = 0; index < ui->stackedWidget->count(); ++index)
    {
        if (auto wid = dynamic_cast<NodeSelectorTreeViewWidget*>(ui->stackedWidget->widget(index)))
        {
            if (!foundTarget && wid->getProxyModel()->getMegaModel() == senderModel)
            {
                foundTarget = true;
                mTargetWid = wid;

                findSource(wid);
            }
            else if (!foundSource)
            {
                findSource(wid);
            }
        }
    }

    targetOrSourceFound(mTargetWid, static_cast<int>(handles.size()));

    for (auto& wid: mSourceWids)
    {
        if (wid != mTargetWid)
        {
            targetOrSourceFound(wid, static_cast<int>(handles.size()));
        }
    }
}

void NodeSelector::onOptionSelected(int index)
{
    switch (index)
    {
        case NodeSelectorTreeViewWidget::TabItem::CLOUD_DRIVE:
        {
            ui->fCloudDrive->setSelected(true);
            break;
        }
        case NodeSelectorTreeViewWidget::TabItem::SHARES:
        {
            ui->fIncomingShares->setSelected(true);
            break;
        }
        case NodeSelectorTreeViewWidget::TabItem::BACKUPS:
        {
            ui->fBackups->setSelected(true);
            break;
        }
        case NodeSelectorTreeViewWidget::TabItem::RUBBISH:
        {
            ui->fRubbish->setSelected(true);
            break;
        }
        default:
        {
            break;
        }
    }
}

void NodeSelector::onCloudDriveTabDropped(std::shared_ptr<QDropEvent> event)
{
    getTreeViewWidget(NodeSelectorTreeViewWidget::TabItem::CLOUD_DRIVE)
        ->dropIntoRootIndex(event.get());
}

void NodeSelector::onbShowCloudDriveClicked()
{
    showTab(NodeSelectorTreeViewWidget::TabItem::CLOUD_DRIVE);
}

void NodeSelector::onbShowRubbishClicked()
{
    showTab(NodeSelectorTreeViewWidget::TabItem::RUBBISH);
}

void NodeSelector::onbShowIncomingSharesClicked()
{
    if (ui->fIncomingShares->isVisible())
    {
        showTab(NodeSelectorTreeViewWidget::TabItem::SHARES);
    }
}

void NodeSelector::onbShowBackupsFolderClicked()
{
    if (ui->fBackups->isVisible())
    {
        showTab(NodeSelectorTreeViewWidget::TabItem::BACKUPS);
    }
}

void NodeSelector::shortCutConnects(int ignoreThis)
{
    // Provide quick access shortcuts for the two panes via Ctrl+1,2
    // Ctrl is auto-magically translated to CMD key by Qt on macOS
    for (int i = 0; i <= NodeSelectorTreeViewWidget::TabItem::BACKUPS; ++i)
    {
        if (i != ignoreThis)
        {
            QShortcut* shortcut =
                new QShortcut(QKeySequence(QString::fromLatin1("Ctrl+%1").arg(i + 1)), this);
            QObject::connect(shortcut,
                             &QShortcut::activated,
                             this,
                             [=]()
                             {
                                 onOptionSelected(i);
                             });
        }
    }
}

void NodeSelector::resetButtonsText()
{
    // Done here to re-use contexts
    ui->bOk->setText(mSelectType->okButtonText());
    ui->bCancel->setText(QCoreApplication::translate("NodeSelectorTreeViewWidget", "Cancel"));
}

void NodeSelector::showTab(NodeSelectorTreeViewWidget::TabItem item)
{
    auto wid = widgetForTab(item);
    if (!wid)
    {
        return;
    }

    // Switching to a real tab while the ghost search tab is shown dismisses the search.
    const bool leavingSearch =
        ui->stackedWidget->currentWidget() == mSearchWidget && wid != mSearchWidget;

    ui->stackedWidget->setCurrentWidget(wid);

    if (leavingSearch)
    {
        mActiveSearchTabType = TabType::NONE;
        if (mSearchWidget)
        {
            mSearchWidget->stopSearch();
            mSearchWidget->resetSearchState();
        }
        ui->leSearchTool->onClearClicked();
    }
}

NodeSelectorTreeViewWidget*
    NodeSelector::widgetForTab(NodeSelectorTreeViewWidget::TabItem item) const
{
    switch (item)
    {
        case NodeSelectorTreeViewWidget::CLOUD_DRIVE:
        {
            return mCloudDriveWidget;
        }
        case NodeSelectorTreeViewWidget::SHARES:
        {
            return mIncomingSharesWidget;
        }
        case NodeSelectorTreeViewWidget::BACKUPS:
        {
            return mBackupsWidget;
        }
        case NodeSelectorTreeViewWidget::RUBBISH:
        {
            return mRubbishWidget;
        }
        case NodeSelectorTreeViewWidget::SEARCH:
        {
            return mSearchWidget;
        }
        default:
        {
            return nullptr;
        }
    }
}

std::optional<NodeSelectorTreeViewWidget::TabItem>
    NodeSelector::tabItemForWidget(const NodeSelectorTreeViewWidget* wid) const
{
    return wid->getTabType();
}

NodeSelectorTreeViewWidget* NodeSelector::widgetForTabType(TabType type) const
{
    switch (type)
    {
        case TabType::CLOUD_DRIVE:
        {
            return mCloudDriveWidget;
        }
        case TabType::INCOMING_SHARE:
        {
            return mIncomingSharesWidget;
        }
        case TabType::BACKUP:
        {
            return mBackupsWidget;
        }
        case TabType::RUBBISH:
        {
            return mRubbishWidget;
        }
        default:
        {
            return nullptr;
        }
    }
}

void NodeSelector::handleSearch(const QString& text)
{
    if (!mSearchWidget || text.isEmpty())
    {
        return;
    }

    // Mark the search in progress so the result-count label shows "Searching…" (not a stale count)
    // for the whole search, regardless of when the loading-scene block signal arrives.
    mSearchInProgress = true;

    // Search spans every chip: leaving the scope unset makes the in-view chips appear.
    mSearchWidget->search(text);
    ui->stackedWidget->setCurrentWidget(mSearchWidget);

    // Refresh now that the search widget is current: the refresh triggered by search() above ran
    // while another tab was still current and hid the label.
    refreshSearchResultCount();
}

void NodeSelector::hideGhostSearch()
{
    if (ui->stackedWidget->currentWidget() == mSearchWidget)
    {
        showTab(mTabBeforeSearch);
    }
}

NodeSelectorTreeViewWidget* NodeSelector::getTreeViewWidget(int page) const
{
    return dynamic_cast<NodeSelectorTreeViewWidget*>(ui->stackedWidget->widget(page));
}

NodeSelectorTreeViewWidget* NodeSelector::getTreeViewWidget(QObject* object) const
{
    return dynamic_cast<NodeSelectorTreeViewWidget*>(object);
}

NodeSelectorTreeViewWidget* NodeSelector::getCurrentTreeViewWidget() const
{
    return getTreeViewWidget(ui->stackedWidget->currentWidget());
}

NodeSelectorTreeViewWidget* NodeSelector::selectedSearchChipTreeViewWidget() const
{
    auto* currentWidget = getCurrentTreeViewWidget();
    if (currentWidget == mSearchWidget && mActiveSearchTabType != TabType::NONE)
    {
        if (auto* diskWidget = widgetForTabType(mActiveSearchTabType))
        {
            return diskWidget;
        }
    }

    return currentWidget;
}

MegaHandle NodeSelector::getSelectedNodeHandle() const
{
    auto currentWidget = getCurrentTreeViewWidget();
    auto selectedHandle =
        currentWidget ? currentWidget->getSelectedNodeHandle() : mega::INVALID_HANDLE;

    return selectedHandle;
}

QList<MegaHandle> NodeSelector::getMultiSelectionNodeHandle()
{
    auto tree_view = getCurrentTreeViewWidget();
    return tree_view ? tree_view->getMultiSelectionNodeHandle() : QList<MegaHandle>();
}

void NodeSelector::closeEvent(QCloseEvent* event)
{
    for (int page = 0; page < ui->stackedWidget->count(); ++page)
    {
        auto viewContainer = getTreeViewWidget(page);
        if (viewContainer)
        {
            viewContainer->abort();
            if (viewContainer->getProxyModel()->isWorking())
            {
                connect(viewContainer->getProxyModel()->getMegaModel(),
                        &NodeSelectorModel::blockUi,
                        this,
                        [this](bool blocked)
                        {
                            if (!blocked)
                            {
                                close();
                            }
                        });
                event->ignore();
                return;
            }
        }
    }

    QDialog::closeEvent(event);
}

std::shared_ptr<MegaNode> NodeSelector::getSelectedNode() const
{
    auto node = std::shared_ptr<MegaNode>(mMegaApi->getNodeByHandle(getSelectedNodeHandle()));
    return node;
}

std::shared_ptr<MegaNode>
    NodeSelector::getNewFolderParentNode(NodeSelectorTreeViewWidget* sourceWidget) const
{
    auto proxyModel = sourceWidget->getProxyModel();
    return proxyModel ? proxyModel->getNode(sourceWidget->getCurrentRootIndex()) : nullptr;
}

void NodeSelector::showNotFoundNodeMessageBox()
{
    MessageDialogInfo msgInfo;
    msgInfo.descriptionText =
        tr("The item you selected has been removed. To reselect, close this window and try again.");
    MessageDialogOpener::warning(msgInfo);
}

void NodeSelector::createActionButtons()
{
    mButtons = {
        {SelectType::ButtonId::UPLOAD, ui->bUpload},
        {SelectType::ButtonId::NEW_FOLDER, ui->bNewFolder},
        {SelectType::ButtonId::CLEAR_RUBBISH, ui->bClearRubbish},
        {SelectType::ButtonId::ADD_BACKUP, ui->bAddBackup},
    };

    for (auto it = mButtons.cbegin(); it != mButtons.cend(); ++it)
    {
        if (it.value())
        {
            connect(it.value(),
                    &QPushButton::clicked,
                    this,
                    [this, id = it.key()]()
                    {
                        onCustomButtonClicked(id);
                    });
        }
    }

    mSelectType->updateActionButtonsText(mButtons);
}

void NodeSelector::initSpecialisedWidgets(NodeSelectorTreeViewWidget* viewContainer)
{
    if (viewContainer)
    {
        viewContainer->setInitialShowLabelText(initialShowLabelText());
        viewContainer->init();

        connect(viewContainer,
                &NodeSelectorTreeViewWidget::onCustomButtonClicked,
                this,
                &NodeSelector::onCustomButtonClicked,
                Qt::UniqueConnection);
        connect(viewContainer,
                &NodeSelectorTreeViewWidget::uiIsBlocked,
                this,
                &NodeSelector::onUiIsBlocked,
                Qt::UniqueConnection);
        connect(viewContainer,
                &NodeSelectorTreeViewWidget::enterKeyPressed,
                ui->bOk,
                &QPushButton::click,
                Qt::UniqueConnection);
        connect(viewContainer,
                &NodeSelectorTreeViewWidget::newFolderRequested,
                this,
                &NodeSelector::onbNewFolderClicked,
                Qt::UniqueConnection);
        connect(viewContainer,
                &NodeSelectorTreeViewWidget::viewReady,
                this,
                &NodeSelector::performNodeSelection);
        connect(viewContainer,
                &NodeSelectorTreeViewWidget::modelModified,
                this,
                &NodeSelector::onModelModified);

        auto model = viewContainer->getProxyModel()->getMegaModel();

        connect(model,
                &NodeSelectorModel::itemsAboutToBeMoved,
                this,
                &NodeSelector::onItemsAboutToBeMoved);

        connect(model,
                &NodeSelectorModel::itemsAboutToBeMovedFailed,
                this,
                &NodeSelector::onItemsAboutToBeMovedFailed);

        connect(model,
                &NodeSelectorModel::itemRequestsFinished,
                this,
                &NodeSelector::onItemRequestsFinished);

        connect(model,
                &NodeSelectorModel::itemsAboutToBeRestored,
                this,
                &NodeSelector::onItemsAboutToBeRestored);

        connect(model,
                &NodeSelectorModel::itemAboutToBeReplaced,
                this,
                &NodeSelector::onItemAboutToBeReplaced);

        connect(model,
                &NodeSelectorModel::itemsAboutToBeMerged,
                this,
                &NodeSelector::onItemsAboutToBeMerged);

        connect(model,
                &NodeSelectorModel::itemMergeFinished,
                this,
                &NodeSelector::onItemMergeFinished);

        connect(model,
                &NodeSelectorModel::itemsAboutToBeMergedFailed,
                this,
                &NodeSelector::onItemsAboutToBeMergedFailed);

        connect(model,
                &NodeSelectorModel::showMessageBox,
                this,
                [this](MessageDialogInfo info)
                {
                    info.parent = this;
                    MessageDialogOpener::warning(info);
                });

        connect(model,
                &NodeSelectorModel::showDuplicatedNodeDialog,
                this,
                [this, model](std::shared_ptr<ConflictTypes> conflicts, MoveActionType type)
                {
                    mDuplicatedModel = model;
                    mDuplicatedConflicts = conflicts;
                    mDuplicatedType = type;

                    auto checkUploadNameDialog = new DuplicatedNodeDialog(this);
                    checkUploadNameDialog->setConflicts(conflicts);

                    DialogOpener::showDialog<DuplicatedNodeDialog, NodeSelector>(
                        checkUploadNameDialog,
                        this,
                        &NodeSelector::onShowDuplicatedNodeDialog);
                });

        ui->stackedWidget->addWidget(viewContainer);

        if (ui->stackedWidget->currentWidget() == viewContainer)
        {
            refreshHeader(viewContainer);
        }
    }
}

bool NodeSelector::eventFilter(QObject* obj, QEvent* event)
{
    if (event->type() == QEvent::DragEnter)
    {
        if (auto button = dynamic_cast<QPushButton*>(obj))
        {
            button->click();
        }
    }

    return QDialog::eventFilter(obj, event);
}

void NodeSelector::setSelectedNodeHandle(std::shared_ptr<MegaNode> node)
{
    mNodeToBeSelected = node;
}

void NodeSelector::performNodeSelection()
{
    if (mNodeToBeSelected)
    {
        std::optional<NodeSelectorTreeViewWidget::TabItem> option = selectedNodeTab();

        if (option.has_value())
        {
            auto optionValue(option.value());
            if (getCurrentTreeViewWidget() == widgetForTab(optionValue))
            {
                onCurrentTreeViewWidgetChanged(ui->stackedWidget->currentIndex());
            }
            else
            {
                showTab(optionValue);
            }
        }

        // Disconnect all connections as soon as the node was selected
        if (!mNodeToBeSelected)
        {
            for (int index = 0; index < ui->stackedWidget->count(); ++index)
            {
                if (auto wid =
                        dynamic_cast<NodeSelectorTreeViewWidget*>(ui->stackedWidget->widget(index)))
                {
                    disconnect(wid,
                               &NodeSelectorTreeViewWidget::viewReady,
                               this,
                               &NodeSelector::performNodeSelection);
                }
            }
        }
    }
}

std::optional<NodeSelectorTreeViewWidget::TabItem> NodeSelector::selectedNodeTab()
{
    if (mNodeToBeSelected)
    {
        std::optional<NodeSelectorTreeViewWidget::TabItem> option;

        if (mMegaApi->isInCloud(mNodeToBeSelected.get()))
        {
            option = NodeSelectorTreeViewWidget::TabItem::CLOUD_DRIVE;
        }
        else if (mMegaApi->isInVault(mNodeToBeSelected.get()))
        {
            option = NodeSelectorTreeViewWidget::TabItem::BACKUPS;
        }
        else if (mMegaApi->isInRubbish(mNodeToBeSelected.get()))
        {
            option = NodeSelectorTreeViewWidget::TabItem::RUBBISH;
        }
        else
        {
            option = NodeSelectorTreeViewWidget::TabItem::SHARES;
        }

        if (option.has_value())
        {
            return option.value();
        }
    }

    return std::nullopt;
}

void NodeSelector::onCurrentTreeViewWidgetChanged(int index)
{
    if (auto wid = dynamic_cast<NodeSelectorTreeViewWidget*>(ui->stackedWidget->widget(index)))
    {
        if (wid == mSearchWidget)
        {
            // Ghost search tab: no entry highlighted in the left sidebar.
            TabSelector::deselectAll(ui->wLeftPaneNS);
        }
        else if (auto tabItem = tabItemForWidget(wid); tabItem.has_value())
        {
            onOptionSelected(static_cast<int>(tabItem.value()));
            // Remember the real tab so the search can be dismissed back to it.
            mTabBeforeSearch = tabItem.value();
        }

        // Disconnect everything BEFORE mutating the selection / root index, so the
        // cascade of signals triggered by clearSelection() and setSelectedNodeHandle()
        // (selectionChanged, viewStateChanged) does not re-enter refreshDestinationBreadcrumb
        // while the proxy model is mid-transition inside loadTreeFromNode/setRootIndex.
        disconnectCurrentWidgetConnections();

        if (mNodeToBeSelected)
        {
            wid->clearSelection();
            wid->setSelectedNodeHandle(mNodeToBeSelected->getHandle());
            mNodeToBeSelected.reset();
        }

        addCurrentWidgetConnection(connect(wid,
                                           &NodeSelectorTreeViewWidget::selectionHasChanged,
                                           this,
                                           &NodeSelector::onSelectionChanged,
                                           Qt::UniqueConnection));
        addCurrentWidgetConnection(connect(wid,
                                           &NodeSelectorTreeViewWidget::viewStateChanged,
                                           this,
                                           [this, wid]()
                                           {
                                               refreshHeader(wid);
                                               refreshNavigationBreadcrumb();
                                           }));
        addCurrentWidgetConnection(connect(wid,
                                           &NodeSelectorTreeViewWidget::currentViewPageChanged,
                                           this,
                                           [this, wid](NodeSelectorTreeViewWidget::ViewType type)
                                           {
                                               if (wid == getCurrentTreeViewWidget())
                                               {
                                                   applySearchToolVisibilityState(wid, type);
                                                   updateHeaderTopRowVisibility();
                                               }
                                           }));
        addCurrentWidgetConnection(connect(wid,
                                           &NodeSelectorTreeViewWidget::viewButtonsStateChanged,
                                           this,
                                           [this, wid]()
                                           {
                                               refreshHeaderButtons(wid);
                                           }));

        updateOkButtonState(wid);
        refreshHeader(wid);
        // The current widget changed (tab switch): its root didn't fire rootIndexChanged,
        // so refresh the navigation breadcrumb explicitly here.
        refreshNavigationBreadcrumb();
        configureTableColumns(wid);
        wid->resetAutoColumnWidths();
    }
}

void NodeSelector::onShowDuplicatedNodeDialog(QPointer<DuplicatedNodeDialog>)
{
    if (mDuplicatedType.has_value())
    {
        mDuplicatedModel->processNodesAfterConflictCheck(
            mDuplicatedConflicts,
            static_cast<MoveActionType>(mDuplicatedType.value()));
        mDuplicatedType = std::nullopt;
        mDuplicatedModel = nullptr;
        mDuplicatedConflicts.reset();
    }
}

void NodeSelector::onNodesUpdate(mega::MegaApi* api, mega::MegaNodeList* nodes)
{
    for (int index = 0; index < ui->stackedWidget->count(); ++index)
    {
        if (auto wid = dynamic_cast<NodeSelectorTreeViewWidget*>(ui->stackedWidget->widget(index)))
        {
            if (wid != mSearchWidget || ui->stackedWidget->currentWidget() == mSearchWidget)
            {
                wid->onNodesUpdate(api, nodes);
            }
        }
    }
}

void NodeSelector::specialisedTreeViewWidgetsCreated()
{
    connectViewConfiguration(mCloudDriveWidget, &NodeSelector::configureTableColumns);

    if (mIncomingSharesWidget)
    {
        connectViewConfiguration(mIncomingSharesWidget, &NodeSelector::configureTableColumns);
        connect(mIncomingSharesWidget,
                &NodeSelectorTreeViewWidget::viewStateChanged,
                this,
                [this]()
                {
                    configureTableColumns(mIncomingSharesWidget);
                });
        connect(mIncomingSharesWidget,
                &NodeSelectorTreeViewWidgetIncomingShares::incomingShareAccessChanged,
                this,
                &NodeSelector::refreshBreadcrumbs);
    }

    connectViewConfiguration(mBackupsWidget, &NodeSelector::configureTableColumns);
    connectViewConfiguration(mRubbishWidget, &NodeSelector::configureTableColumns);

    if (mSearchWidget)
    {
        connect(mSearchWidget,
                &NodeSelectorTreeViewWidget::viewReady,
                this,
                [this]()
                {
                    configureSearchWidget(TabType::NONE);
                });
        // Column visibility is no longer buffered by the loading scene; re-apply it (for the
        // active search chip) whenever the search model is reattached after a loading cycle.
        connect(mSearchWidget,
                &NodeSelectorTreeViewWidget::modelReattached,
                this,
                [this]()
                {
                    configureSearchWidget(mActiveSearchTabType);
                });
        connect(mSearchWidget,
                &NodeSelectorTreeViewWidgetSearch::searchTabTypeChanged,
                this,
                [this](TabType type)
                {
                    // Track which chip is selected so the destination breadcrumb and
                    // chip-specific banners resolve against the right tab while searching.
                    mActiveSearchTabType = type;
                    configureSearchWidget(type);
                    refreshBreadcrumbs();
                });
    }
}

void NodeSelector::connectViewConfiguration(NodeSelectorTreeViewWidget* widget,
                                            ViewConfigurationFunction configure)
{
    if (!widget)
    {
        return;
    }

    // Run for ALL widgets (not just the current one): each widget owns its column visibility and
    // re-applies it whenever its own model is reattached after a loading cycle.
    const auto refreshColumns = [this, configure](NodeSelectorTreeViewWidget* wid)
    {
        if (wid)
        {
            (this->*configure)(wid);
        }
    };

    connect(widget, &NodeSelectorTreeViewWidget::viewReady, this, refreshColumns);
    // The loading scene no longer buffers column-hidden changes; re-apply column visibility every
    // time the model is reattached (the header is reset by setModel/restoreState on reattach).
    connect(widget, &NodeSelectorTreeViewWidget::modelReattached, this, refreshColumns);
    // Permanent (not per-current-widget) so it exists before the first setRootIndex.
    // refreshNavigationBreadcrumb resolves against the current widget, so a non-current
    // widget firing this is harmless.
    connect(widget,
            &NodeSelectorTreeViewWidget::rootIndexChanged,
            this,
            &NodeSelector::refreshNavigationBreadcrumb,
            Qt::UniqueConnection);
    connect(widget,
            &NodeSelectorTreeViewWidget::nodesRenamed,
            this,
            &NodeSelector::onNodesRenamed,
            Qt::UniqueConnection);
    // After a deferred root commit (first entry into a not-yet-cached folder), the header and
    // column visibility must be recomputed against the now-valid committed root.
    connect(
        widget,
        &NodeSelectorTreeViewWidget::rootIndexChanged,
        this,
        [this, widget, configure]()
        {
            if (widget == getCurrentTreeViewWidget())
            {
                (this->*configure)(widget);
                refreshHeader(widget);
            }
        },
        Qt::UniqueConnection);
}

void NodeSelector::configureTableColumns(NodeSelectorTreeViewWidget* widget)
{
    if (!widget)
    {
        return;
    }

    widget->setColumnHidden(NodeSelectorModel::Column::NODE, false);
    widget->setColumnHidden(NodeSelectorModel::Column::LABEL, false);
    widget->setColumnHidden(NodeSelectorModel::Column::IS_EXPORTED,
                            (widget->getTabType() == NodeSelectorTreeViewWidget::TabItem::RUBBISH ||
                             widget->getTabType() == NodeSelectorTreeViewWidget::TabItem::SHARES));

    // Show incoming shares when we are in the incoming shares tab and the root index
    setIncomingShareColumnsVisibility(widget,
                                      widget->getTabType() ==
                                              NodeSelectorTreeViewWidget::TabItem::SHARES &&
                                          !widget->getCurrentRootIndex().isValid());
    configureTypeSpecificColumns(widget);
}

void NodeSelector::configureTypeSpecificColumns(NodeSelectorTreeViewWidget* widget)
{
    Q_UNUSED(widget)
}

void NodeSelector::setIncomingShareColumnsVisibility(NodeSelectorTreeViewWidget* widget,
                                                     bool visible)
{
    if (!widget)
    {
        return;
    }

    widget->setColumnHidden(NodeSelectorModel::Column::USER, !visible);
    widget->setColumnHidden(NodeSelectorModel::Column::ACCESS, !visible);
}

void NodeSelector::configureSearchWidget(TabType type)
{
    if (!mSearchWidget)
    {
        return;
    }

    mSearchWidget->setColumnHidden(NodeSelectorModel::Column::NODE, false);
    mSearchWidget->setColumnHidden(NodeSelectorModel::Column::LABEL, false);
    mSearchWidget->setColumnHidden(NodeSelectorModel::Column::IS_EXPORTED,
                                   (type == TabType::RUBBISH || type == TabType::INCOMING_SHARE));

    // Mirror the Incoming Shares tab columns while its search chip is active.
    setIncomingShareColumnsVisibility(mSearchWidget, type == TabType::INCOMING_SHARE);
    configureTypeSpecificColumns(mSearchWidget);

    mSearchWidget->resetAutoColumnWidths();
}

void NodeSelector::createSpecialisedTreeViewWidgets()
{
    const QList<QPair<TabType, TabSelector*>> tabOrder = {
        {TabType::CLOUD_DRIVE, ui->fCloudDrive},
        {TabType::INCOMING_SHARE, ui->fIncomingShares},
        {TabType::BACKUP, ui->fBackups},
        {TabType::RUBBISH, ui->fRubbish},
    };

    auto allowed = mSelectType->allowedTabTypes();

    for (const auto& [type, button]: tabOrder)
    {
        if (allowed & type)
        {
            button->show();
            auto wid = addWidgetForTabType(type);
            if (wid)
            {
                initSpecialisedWidgets(wid);
            }
        }
        else
        {
            button->hide();
        }
    }
}

NodeSelectorTreeViewWidget* NodeSelector::addWidgetForTabType(TabType type)
{
    switch (type)
    {
        case TabType::CLOUD_DRIVE:
        {
            return addCloudDrive();
        }
        case TabType::INCOMING_SHARE:
        {
            return addIncomingShares();
        }
        case TabType::BACKUP:
        {
            return addBackups();
        }
        case TabType::RUBBISH:
        {
            return addRubbish();
        }
        default:
        {
            return nullptr;
        }
    }
}

NodeSelectorTreeViewWidget* NodeSelector::addCloudDrive()
{
    mCloudDriveWidget = new NodeSelectorTreeViewWidgetCloudDrive(mSelectType);
    mCloudDriveWidget->setObjectName(QString::fromUtf8("CloudDrive"));
    return mCloudDriveWidget;
}

NodeSelectorTreeViewWidget* NodeSelector::addIncomingShares()
{
    mIncomingSharesWidget = new NodeSelectorTreeViewWidgetIncomingShares(mSelectType);
    mIncomingSharesWidget->setObjectName(QString::fromUtf8("IncomingShares"));
    return mIncomingSharesWidget;
}

NodeSelectorTreeViewWidget* NodeSelector::addBackups()
{
    mBackupsWidget = new NodeSelectorTreeViewWidgetBackups(mSelectType);
    mBackupsWidget->setObjectName(QString::fromUtf8("Backups"));
    return mBackupsWidget;
}

NodeSelectorTreeViewWidget* NodeSelector::addSearchTreeViewWidget()
{
    mSearchWidget = new NodeSelectorTreeViewWidgetSearch(mSelectType);
    mSearchWidget->setObjectName(QString::fromUtf8("Search"));
    initSpecialisedWidgets(mSearchWidget);
    mSearchWidget->prepareForInitialDisplay();
    connect(mSearchWidget,
            &NodeSelectorTreeViewWidgetSearch::nodeDoubleClicked,
            this,
            [this](std::shared_ptr<MegaNode> node)
            {
                setSelectedNodeHandle(node);
                performNodeSelection();
            });
    return mSearchWidget;
}

NodeSelectorTreeViewWidget* NodeSelector::addRubbish()
{
    mRubbishWidget = new NodeSelectorTreeViewWidgetRubbish(mSelectType);
    mRubbishWidget->setObjectName(QString::fromUtf8("Rubbish"));
    return mRubbishWidget;
}

void NodeSelector::onCustomButtonClicked(uint id)
{
    if (id == CloudDriveType::NEW_FOLDER)
    {
        onbNewFolderClicked();
    }
}
