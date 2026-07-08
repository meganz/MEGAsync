#include "FileManagerNodeSelector.h"

#include "CreateRemoveBackupsManager.h"
#include "DialogOpener.h"
#include "megaapi.h"
#include "MessageDialogOpener.h"
#include "NavigationBreadcrumb.h"
#include "NodeSelectorModel.h"
#include "NodeSelectorTreeViewWidgetSpecializations.h"
#include "Preferences.h"
#include "RequestListenerManager.h"
#include "StatsEventHandler.h"
#include "TabSelector.h"
#include "tokenizer/TokenizableItems/TokenizableButtons.h"
#include "ui_NodeSelector.h"

#include <QBoxLayout>
#include <QMessageBox>
#include <QStyle>

FileManagerNodeSelector::FileManagerNodeSelector(QWidget* parent):
    NodeSelector(SelectTypeSPtr(new CloudDriveType), parent)
{
    ui->bOk->hide();
    ui->bCancel->hide();

    mDragBackDrop = new QWidget(this);
    mDragBackDrop->hide();

    ui->destinationBreadcrumb->setVisible(false);

    connect(ui->navigationBreadcrumb,
            &NavigationBreadcrumb::segmentActivated,
            this,
            &FileManagerNodeSelector::onNavigationBreadcrumbSegmentActivated);
    connect(ui->navigationBreadcrumb,
            &NavigationBreadcrumb::lastSegmentMenuRequested,
            this,
            &FileManagerNodeSelector::onNavigationBreadcrumbMenuRequested);
    connect(ui->navigationBreadcrumb,
            &NavigationBreadcrumb::refreshNeeded,
            this,
            &FileManagerNodeSelector::refreshNavigationBreadcrumb);
    // Qualified call: intentionally non-virtual during construction.
    FileManagerNodeSelector::refreshNavigationBreadcrumb();

    setAcceptDrops(true);

#ifndef Q_OS_MACOS
    Qt::WindowFlags flags = Qt::Window;
    this->setWindowFlags(flags);
#ifdef Q_OS_LINUX
    this->setWindowFlags(this->windowFlags());
#endif
#endif

    // Update last time opened
    Preferences::instance()->cloudDriveDialogOpened();

    // Send stats in case we didn´t send them in the current hour
    sendStats();
}

void FileManagerNodeSelector::refreshNavigationBreadcrumb()
{
    auto* breadcrumbWidget = getCurrentTreeViewWidget();

    // The ghost search tab shows a flat result list across every chip, so the per-tab navigation
    // breadcrumb does not apply while it is visible.
    const bool shouldShowBreadcrumb = breadcrumbWidget && breadcrumbWidget != mSearchWidget;

    ui->navigationBreadcrumb->setVisible(shouldShowBreadcrumb);
    if (!shouldShowBreadcrumb)
    {
        ui->navigationBreadcrumb->setSegments({});
        return;
    }

    // Inside a folder the last segment always keeps its chevron + menu. At the top root it is
    // only interactive when the root has items and is not read-only; otherwise it is rendered
    // plain (the empty page / read-only root offers no folder actions there).
    const bool lastSegmentInteractive =
        !breadcrumbWidget->isCurrentRootIndexReadOnly() &&
        (!breadcrumbWidget->isAtTopRoot() || !breadcrumbWidget->isTopRootEmpty());

    ui->navigationBreadcrumb->setSegments(breadcrumbWidget->navigationBreadcrumbSegments(),
                                          lastSegmentInteractive);
}

void FileManagerNodeSelector::onNodesRenamed(const QList<mega::MegaHandle>& handles)
{
    ui->navigationBreadcrumb->onNodesRenamed(handles);
}

void FileManagerNodeSelector::onNavigationBreadcrumbSegmentActivated(int segmentIndex)
{
    if (auto* widget = getCurrentTreeViewWidget(); widget && widget != mSearchWidget)
    {
        widget->navigateToBreadcrumbSegment(segmentIndex);
    }
}

void FileManagerNodeSelector::onNavigationBreadcrumbMenuRequested(const QPoint& globalPos)
{
    if (auto* widget = getCurrentTreeViewWidget(); widget && widget != mSearchWidget)
    {
        widget->showCurrentRootContextMenu(globalPos);
    }
}

void FileManagerNodeSelector::specialisedTreeViewWidgetsCreated()
{
    NodeSelector::specialisedTreeViewWidgetsCreated();

    if (mSearchWidget)
    {
        connect(mSearchWidget,
                &NodeSelectorTreeViewWidgetSearch::searchCounterChanged,
                this,
                &FileManagerNodeSelector::refreshSearchResultCount);
    }
}

void FileManagerNodeSelector::refreshSearchResultCount()
{
    if (!mSearchWidget || ui->stackedWidget->currentWidget() != mSearchWidget)
    {
        ui->lSearchResultCount->setVisible(false);
        ui->lSearchResultCount->clear();
        return;
    }

    // While a search/load is in progress, show a "searching" indicator instead of the count: the
    // model is being repopulated and any intermediate value (including "0 result") is misleading.
    // The final count is shown once it finishes.
    // UI blocks coming from the moving-nodes accounting (deleting/moving items from the results)
    // are not searches: keep showing the count, which updates as the rows are removed.
    const auto searchMegaModel = mSearchWidget->getProxyModel()->getMegaModel();
    const bool blockedByNodeMoves = searchMegaModel && searchMegaModel->isMovingNodes();
    if (isSearchInProgress() || (isUiBlocked() && !blockedByNodeMoves))
    {
        showSearchingIndicator();
        return;
    }

    const int count = mSearchWidget->searchResultCount();
    ui->lSearchResultCount->setText(tr("%n result", "", count));
    ui->lSearchResultCount->setEnabled(true);
    ui->lSearchResultCount->setVisible(true);
}

void FileManagerNodeSelector::configureTypeSpecificColumns(NodeSelectorTreeViewWidget* widget)
{
    if (!widget)
    {
        return;
    }

    // Hide the date columns on the Incoming Shares top-level list (the share roots), where
    // USER/ACCESS are shown instead. They remain visible everywhere else.
    const bool incomingSharesTopRoot =
        widget->getTabType() == NodeSelectorTreeViewWidget::TabItem::SHARES &&
        !widget->getCurrentRootIndex().isValid();

    // The search view mirrors the Incoming Shares tab columns while its chip is active.
    const bool searchingIncomingShares =
        widget == mSearchWidget && mActiveSearchTabType == TabType::INCOMING_SHARE;

    widget->setColumnHidden(NodeSelectorModel::Column::ADDED_DATE,
                            incomingSharesTopRoot || searchingIncomingShares);
    widget->setColumnHidden(NodeSelectorModel::Column::LAST_MODIFIED_DATE,
                            incomingSharesTopRoot || searchingIncomingShares);
}

void FileManagerNodeSelector::configureSidebar()
{
    static constexpr int EXPANDED_SIDEBAR_WIDTH = 256;
    static constexpr int EXPANDED_TAB_HEIGHT = 40;
    static constexpr int EXPANDED_ICON_SIZE = 24;

    ui->wLeftPaneNS->setMinimumWidth(EXPANDED_SIDEBAR_WIDTH);
    ui->wLeftPaneNS->setMaximumWidth(EXPANDED_SIDEBAR_WIDTH);

    const auto expandTab = [](TabSelector* tab)
    {
        if (!tab)
        {
            return;
        }
        tab->setIconOnly(false);
        tab->setProperty("class", QLatin1String("sidebar"));
        tab->setMinimumHeight(EXPANDED_TAB_HEIGHT);
        tab->setMaximumHeight(EXPANDED_TAB_HEIGHT);
        tab->setIconSize(QSize(EXPANDED_ICON_SIZE, EXPANDED_ICON_SIZE));
        tab->style()->unpolish(tab);
        tab->style()->polish(tab);
    };

    expandTab(ui->fCloudDrive);
    expandTab(ui->fIncomingShares);
    expandTab(ui->fBackups);
    expandTab(ui->fRubbish);

    // FileManager left pane: 12px right margin on the tab list (left already comes as 12 from
    // the content layout). Set on TabsLayout only, so wAccountInfo is not affected.
    ui->TabsLayout->setContentsMargins(0, 0, 12, 4);

    resize(1024, 720);
    setMinimumSize(1024, 691);
}

void FileManagerNodeSelector::configureHeader()
{
    // Search Line Edit
    ui->leSearchTool->setMode(SearchLineEdit::Mode::ALWAYS_EXPANDED);

    static constexpr int SEARCH_LINE_EDIT_FIXED_WIDTH = 345;
    static constexpr int SEARCH_LINE_EDIT_FIXED_HEIGHT = 40;

    ui->leSearchTool->setFixedSize(SEARCH_LINE_EDIT_FIXED_WIDTH, SEARCH_LINE_EDIT_FIXED_HEIGHT);
}

void FileManagerNodeSelector::configureActionButtonsPlacement()
{
    const auto applyHeaderStyle = [](TokenizableButton* btn, const QString& type)
    {
        if (!btn)
        {
            return;
        }
        btn->setProperty("type", type);
        btn->setProperty("dimension", QLatin1String("medium"));
        btn->style()->unpolish(btn);
        btn->style()->polish(btn);
    };

    applyHeaderStyle(ui->bUpload, QLatin1String("primary"));
    applyHeaderStyle(ui->bClearRubbish, QLatin1String("primary"));
    applyHeaderStyle(ui->bNewFolder, QLatin1String("secondary"));
    applyHeaderStyle(ui->bAddBackup, QLatin1String("primary"));

    for (auto* btn: {ui->bUpload, ui->bNewFolder, ui->bClearRubbish, ui->bAddBackup})
    {
        ui->actionButtonsLayout->addWidget(btn);
    }

    ui->actionButtonsLayout->addStretch();
}

void FileManagerNodeSelector::configureFooter()
{
    ui->footer->setVisible(false);
    // The footer divider is only shown in the file picker;
    ui->footerDivider->setVisible(false);
}

void FileManagerNodeSelector::onLanguageChangeEvent() {}

void FileManagerNodeSelector::applyNewFolderSelection(NodeSelectorTreeViewWidget* sourceWidget,
                                                      mega::MegaNode* newNode)
{
    // Mark the new folder so the view navigates into it once it is added to the model.
    sourceWidget->setNewFolderInfo({newNode->getHandle(), true});
}

void FileManagerNodeSelector::sendStats()
{
    auto lastDateTimeStatSent(Preferences::instance()->cloudDriveDialogLastDateTimeStatSent());
    auto lastDateTimeOpened(Preferences::instance()->cloudDriveDialogLastDateTimeOpened());

    // If event sent during the current hour, return
    if (!Utilities::hourHasChanged(lastDateTimeStatSent, QDateTime::currentSecsSinceEpoch()))
    {
        return;
    }

    auto sendEvent = []()
    {
        MegaSyncApp->getStatsEventHandler()->sendEvent(
            AppStatsEvents::EventType::CLOUD_DRIVE_HOURLY_ACTIVE_USERS);

        Preferences::instance()->updateCloudDriveDialogLastDateTimeStatSent();
    };

    // If the CloudDrive is currently open, send the event
    // If the CloudDrive is not currently open but it was opened during the current hour, send
    // the event
    auto cloudDriveDialog(DialogOpener::findDialog<NodeSelector>());
    auto isCurrentlyOpen(cloudDriveDialog && dynamic_cast<FileManagerNodeSelector*>(
                                                 cloudDriveDialog->getDialog().data()));

    if (isCurrentlyOpen)
    {
        sendEvent();
    }
    else
    {
        // If the hour between now and the last date time opened has changed, it is not the
        // current hour
        auto openedDuringCurrentHour(
            !(Utilities::hourHasChanged(QDateTime::currentSecsSinceEpoch(), lastDateTimeOpened)));
        if (openedDuringCurrentHour)
        {
            sendEvent();
        }
    }
}

void FileManagerNodeSelector::onCustomButtonClicked(uint id)
{
    if (id == SelectType::UPLOAD)
    {
        MegaSyncApp->getStatsEventHandler()->sendTrackedEvent(
            AppStatsEvents::EventType::CLOUD_DRIVE_UPLOAD_CLICKED);
        auto currentTreeWidget = getCurrentTreeViewWidget();
        auto rootItem = currentTreeWidget ? currentTreeWidget->rootItem() : nullptr;
        if (rootItem)
        {
            auto node = rootItem->getNode();
            if (node)
            {
                MegaSyncApp->runUploadActionWithTargetHandle(node->getHandle(),
                                                             mega::MegaApi::PITAG_TRIGGER_PICKER,
                                                             this);
            }
            else
            {
                showNotFoundNodeMessageBox();
            }
        }
    }
    else if (id == SelectType::CLEAR_RUBBISH)
    {
        MessageDialogInfo msgInfo;
        msgInfo.parent = this;
        msgInfo.buttons = QMessageBox::Yes | QMessageBox::No;
        QMap<QMessageBox::Button, QString> textsByButton;
        textsByButton.insert(QMessageBox::Yes, tr("Empty"));
        textsByButton.insert(QMessageBox::No, tr("Cancel"));
        msgInfo.buttonsText = textsByButton;
        msgInfo.defaultButton = QMessageBox::No;
        msgInfo.textFormat = Qt::RichText;
        msgInfo.titleText = tr("Empty Rubbish bin?");
        msgInfo.descriptionText =
            tr("All items will be permanently deleted. This action can [B]not[/B] be undone");
        msgInfo.finishFunc = [this](QPointer<MessageDialogResult> msg)
        {
            if (msg->result() == QMessageBox::Yes)
            {
                mRubbishWidget->setLoadingSceneVisible(true);
                // cleanRubbishBin does not go through the moving-nodes accounting that
                // emits blockUi(false), so hide the loading scene when the request finishes;
                // otherwise the view stays stuck in the loading scene forever.
                auto listener =
                    RequestListenerManager::instance().registerAndGetCustomFinishListener(
                        this,
                        [this](mega::MegaRequest*, mega::MegaError*)
                        {
                            mRubbishWidget->setLoadingSceneVisible(false);
                        });
                MegaSyncApp->getMegaApi()->cleanRubbishBin(listener.get());
            }
        };
        MessageDialogOpener::warning(msgInfo);
    }
    else if (id == SelectType::ADD_BACKUP)
    {
        CreateRemoveBackupsManager::addBackup(SyncInfo::SyncOrigin::CLOUD_DRIVE_DIALOG_ORIGIN,
                                              QStringList(),
                                              this);
    }

    NodeSelector::onCustomButtonClicked(id);
}

void FileManagerNodeSelector::onItemsAboutToBeMoved(const QList<mega::MegaHandle>& handles,
                                                    int type)
{
    checkMovingItems(handles, type, NodeSelector::IncreaseOrDecrease::INCREASE);
}

void FileManagerNodeSelector::onItemsAboutToBeMovedFailed(const QList<mega::MegaHandle>& handles,
                                                          int type)
{
    checkMovingItems(handles, type, NodeSelector::IncreaseOrDecrease::DECREASE);
}

void FileManagerNodeSelector::onItemRequestsFinished(int actionType)
{
    if (actionType == MoveActionType::DELETE_RUBBISH && mRubbishWidget->isUiBlocked())
    {
        mRubbishWidget->finishMovingNodes();
    }
}

void FileManagerNodeSelector::onItemsAboutToBeRestored(const QSet<mega::MegaHandle>& handles)
{
    auto tabsInfo(getTabs(handles.values()));

    if (!tabsInfo.cloudDriveNodes.isEmpty())
    {
        mCloudDriveWidget->setParentOfRestoredNodes(handles);
    }

    if (!tabsInfo.incomingSharedNodes.isEmpty())
    {
        mIncomingSharesWidget->setParentOfRestoredNodes(handles);
    }
}

void FileManagerNodeSelector::onItemAboutToBeReplaced(mega::MegaHandle handle)
{
    auto tabsInfo(getTabs(QList<mega::MegaHandle>() << handle));

    if (!tabsInfo.cloudDriveNodes.isEmpty())
    {
        mCloudDriveWidget->addHandleToBeReplaced(handle);
    }

    if (!tabsInfo.incomingSharedNodes.isEmpty())
    {
        mIncomingSharesWidget->addHandleToBeReplaced(handle);
    }
}

void FileManagerNodeSelector::onItemsAboutToBeMerged(
    const QList<std::shared_ptr<NodeSelectorMergeInfo>>& mergesInfo,
    int actionType)
{
    performMergeAction(mergesInfo, actionType, NodeSelector::IncreaseOrDecrease::INCREASE);
}

void FileManagerNodeSelector::onItemMergeFinished(mega::MegaHandle sourceHandle,
                                                  mega::MegaHandle targetHandle,
                                                  int)
{
    const auto targetTabsInfo(getTabs(QList<mega::MegaHandle>() << targetHandle));

    auto finishMergeInWidget =
        [sourceHandle, targetHandle](const QList<mega::MegaHandle>& targetHandlesByTab,
                                     NodeSelectorTreeViewWidget* wid)
    {
        if (!wid || !targetHandlesByTab.contains(targetHandle))
        {
            return;
        }

        QMultiHash<NodeSelectorTreeViewWidget::SourceHandle,
                   NodeSelectorTreeViewWidget::TargetHandle>
            merges;
        merges.insert(sourceHandle, targetHandle);

        wid->decreaseMovingNodes(1);
        wid->resetMergeFolderHandles(merges);
    };

    finishMergeInWidget(targetTabsInfo.cloudDriveNodes, mCloudDriveWidget);
    finishMergeInWidget(targetTabsInfo.incomingSharedNodes, mIncomingSharesWidget);
}

void FileManagerNodeSelector::onItemsAboutToBeMergedFailed(
    const QList<std::shared_ptr<NodeSelectorMergeInfo>>& mergesInfo,
    int actionType)
{
    performMergeAction(mergesInfo, actionType, NodeSelector::IncreaseOrDecrease::DECREASE);
}

void FileManagerNodeSelector::performMergeAction(
    const QList<std::shared_ptr<NodeSelectorMergeInfo>>& mergesInfo,
    int actionType,
    IncreaseOrDecrease type)
{
    QList<NodeSelectorTreeViewWidget::SourceHandle> sourceHandles;
    QList<NodeSelectorTreeViewWidget::TargetHandle> targetHandles;
    for (const auto& info: mergesInfo)
    {
        sourceHandles.append(info->nodeToMerge->getHandle());
        targetHandles.append(info->nodeTarget->getHandle());
    }

    if (actionType != MoveActionType::COPY)
    {
        performItemsToBeMoved(sourceHandles,
                              NodeSelector::IncreaseOrDecrease::INCREASE,
                              true,
                              false);
    }

    auto fillMergeFolders =
        [type,
         &mergesInfo](const QList<NodeSelectorTreeViewWidget::TargetHandle>& targetHandlesByTab,
                      NodeSelectorTreeViewWidget* wid)
    {
        QMultiHash<NodeSelectorTreeViewWidget::SourceHandle,
                   NodeSelectorTreeViewWidget::TargetHandle>
            merges;
        for (const auto& info: mergesInfo)
        {
            if (targetHandlesByTab.contains(info->nodeTarget->getHandle()))
            {
                merges.insert(info->nodeToMerge->getHandle(), info->nodeTarget->getHandle());
            }
        }

        if (type == NodeSelector::IncreaseOrDecrease::INCREASE)
        {
            wid->increaseMovingNodes(static_cast<int>(merges.size()));
            wid->setMergeFolderHandles(merges);
        }
        else
        {
            wid->decreaseMovingNodes(static_cast<int>(merges.size()));
            wid->resetMergeFolderHandles(merges);
        }
    };

    auto targetTabsInfo(getTabs(targetHandles));

    if (!targetTabsInfo.cloudDriveNodes.isEmpty())
    {
        if (actionType != MoveActionType::COPY)
        {
            fillMergeFolders(targetTabsInfo.cloudDriveNodes, mCloudDriveWidget);
        }
        else
        {
            mCloudDriveWidget->setAsyncSelectedNodeHandle(targetTabsInfo.cloudDriveNodes);
            mCloudDriveWidget->selectPendingIndexes();
        }

        if (type == NodeSelector::IncreaseOrDecrease::INCREASE &&
            actionType == MoveActionType::RESTORE)
        {
            // Check with the handle if we are in CD or Incoming
            if (ui->stackedWidget->currentWidget() == mRubbishWidget)
            {
                onbShowCloudDriveClicked();
            }
        }
    }

    if (!targetTabsInfo.incomingSharedNodes.isEmpty())
    {
        if (actionType != MoveActionType::COPY)
        {
            fillMergeFolders(targetTabsInfo.incomingSharedNodes, mIncomingSharesWidget);
        }
        else
        {
            mIncomingSharesWidget->setAsyncSelectedNodeHandle(targetTabsInfo.incomingSharedNodes);
            mIncomingSharesWidget->selectPendingIndexes();
        }

        if (type == NodeSelector::IncreaseOrDecrease::INCREASE &&
            actionType == MoveActionType::RESTORE)
        {
            // Check with the handle if we are in CD or Incoming
            if (ui->stackedWidget->currentWidget() == mRubbishWidget)
            {
                onbShowIncomingSharesClicked();
            }
        }
    }
}

void FileManagerNodeSelector::checkMovingItems(const QList<mega::MegaHandle>& handles,
                                               int moveType,
                                               NodeSelector::IncreaseOrDecrease type)
{
    if (moveType == MoveActionType::RESTORE)
    {
        auto tabsInfo(getTabs(handles));

        if (!tabsInfo.cloudDriveNodes.isEmpty())
        {
            type == NodeSelector::IncreaseOrDecrease::INCREASE ?
                mCloudDriveWidget->increaseMovingNodes(
                    static_cast<int>(tabsInfo.cloudDriveNodes.size())) :
                mCloudDriveWidget->decreaseMovingNodes(
                    static_cast<int>(tabsInfo.cloudDriveNodes.size()));
        }

        if (!tabsInfo.incomingSharedNodes.isEmpty())
        {
            type == NodeSelector::IncreaseOrDecrease::INCREASE ?
                mIncomingSharesWidget->increaseMovingNodes(
                    static_cast<int>(tabsInfo.incomingSharedNodes.size())) :
                mIncomingSharesWidget->decreaseMovingNodes(
                    static_cast<int>(tabsInfo.incomingSharedNodes.size()));
        }

        selectTabs(tabsInfo);

        performItemsToBeMoved(handles, type, true, false);
    }
    else if (moveType == MoveActionType::DELETE_RUBBISH)
    {
        if (type == NodeSelector::IncreaseOrDecrease::INCREASE)
        {
            mRubbishWidget->increaseMovingNodes(static_cast<int>(handles.size()));
        }
        else
        {
            mRubbishWidget->decreaseMovingNodes(static_cast<int>(handles.size()));
        }

        performItemsToBeMoved(handles, type, true, false);
    }
    else if (moveType == MoveActionType::DELETE_PERMANENTLY)
    {
        performItemsToBeMoved(handles, type, true, false);
    }
    else if (moveType == MoveActionType::COPY)
    {
        performItemsToBeMoved(handles, type, false, true);
    }
    else
    {
        performItemsToBeMoved(handles, type, true, true);
    }
}

FileManagerNodeSelector::HandlesByTab
    FileManagerNodeSelector::getTabs(const QList<mega::MegaHandle>& handles)
{
    HandlesByTab info;

    if (!handles.isEmpty())
    {
        for (const auto& handle: handles)
        {
            std::unique_ptr<mega::MegaNode> node(
                MegaSyncApp->getMegaApi()->getNodeByHandle(handle));
            if (node)
            {
                mega::MegaNode* checkNode(nullptr);

                std::unique_ptr<mega::MegaNode> restoreNode(
                    MegaSyncApp->getMegaApi()->getNodeByHandle(node->getRestoreHandle()));
                if (restoreNode)
                {
                    checkNode = restoreNode.get();
                }
                else
                {
                    checkNode = node.get();
                }

                if (MegaSyncApp->getMegaApi()->isInCloud(checkNode))
                {
                    info.cloudDriveNodes.append(handle);
                }
                else
                {
                    info.incomingSharedNodes.append(handle);
                }
            }
        }
    }

    return info;
}

void FileManagerNodeSelector::selectTabs(const HandlesByTab& tabsInfo)
{
    if (!tabsInfo.cloudDriveNodes.isEmpty())
    {
        onbShowCloudDriveClicked();
        onOptionSelected(NodeSelectorTreeViewWidget::TabItem::CLOUD_DRIVE);
    }
    else if (!tabsInfo.incomingSharedNodes.isEmpty())
    {
        onbShowIncomingSharesClicked();
        onOptionSelected(NodeSelectorTreeViewWidget::TabItem::SHARES);
    }
}
