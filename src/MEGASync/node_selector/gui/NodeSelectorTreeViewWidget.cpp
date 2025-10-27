#include "NodeSelectorTreeViewWidget.h"

#include "DialogOpener.h"
#include "EventUpdater.h"
#include "MegaApplication.h"
#include "MegaNodeNames.h"
#include "MessageDialogOpener.h"
#include "NewFolderDialog.h"
#include "NodeSelectorDelegates.h"
#include "NodeSelectorModel.h"
#include "NodeSelectorProxyModel.h"
#include "NodeSelectorTreeViewWidgetSpecializations.h"
#include "RenameNodeDialog.h"
#include "RequestListenerManager.h"
#include "TokenizableItems/TokenPropertySetter.h"
#include "ui_NodeSelectorTreeViewWidget.h"

const int CHECK_UPDATED_NODES_INTERVAL = 1000;
const int IMMEDIATE_CHECK_UPDATES_NODES_THRESHOLD = 200;

NodeSelectorTreeViewWidget::NodeSelectorTreeViewWidget(SelectTypeSPtr mode, QWidget* parent):
    QWidget(parent),
    ui(new Ui::NodeSelectorTreeViewWidget),
    mProxyModel(nullptr),
    mModel(nullptr),
    mMegaApi(MegaSyncApp->getMegaApi()),
    mSelectType(mode),
    mManuallyResizedColumn(false),
    mResizeEventsReceived(0),
    first(true),
    mUiBlocked(false),
    mNewFolderHandle(mega::INVALID_HANDLE),
    mNewFolderAdded(false)
{
    ui->setupUi(this);
    setFocusProxy(ui->tMegaFolders);
    ui->bOk->setDefault(true);
    ui->bOk->setEnabled(false);
    ui->searchButtonsWidget->setVisible(false);

    connect(ui->bNewFolder,
            &QPushButton::clicked,
            this,
            &NodeSelectorTreeViewWidget::onbNewFolderClicked);
    connect(ui->bOk, &QPushButton::clicked, this, &NodeSelectorTreeViewWidget::okBtnClicked);
    connect(ui->bCancel,
            &QPushButton::clicked,
            this,
            &NodeSelectorTreeViewWidget::cancelBtnClicked);

    checkBackForwardButtons();
    checkOkCancelButtonsVisibility();
    addCustomButtons(this);

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

    ui->cloudDriveSearch->setProperty("title", MegaNodeNames::getCloudDriveName());
    ui->backupsSearch->setProperty("title", MegaNodeNames::getBackupsName());
    ui->incomingSharesSearch->setProperty("title", MegaNodeNames::getIncomingSharesName());
    ui->rubbishSearch->setProperty("title", MegaNodeNames::getRubbishName());

    ui->footer->setVisible(mSelectType->footerVisible());
    ui->incomingInfo->setVisible(false);

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
}

NodeSelectorTreeViewWidget::~NodeSelectorTreeViewWidget()
{
    delete ui;
}

void NodeSelectorTreeViewWidget::init()
{
    mProxyModel = createProxyModel();
    mModel = createModel();
    // Regardless the type of treeviewwidget, the empty icon always use icon-secondary token
    ui->emptyIcon->setProperty(TOKEN_PROPERTIES::normalOff, QLatin1String("icon-secondary"));
    ui->emptyIcon->setIcon(getEmptyIcon());

    initEmptyMessages();

    ui->emptyPage->installEventFilter(this);
    mSelectType->init(this);

    ui->tMegaFolders->setSortingEnabled(true);
    ui->tMegaFolders->setAllowContextMenu(mSelectType->isContextMenuAllowed());
    ui->tMegaFolders->viewport()->installEventFilter(this);
    ui->emptyFolderPage->installEventFilter(this);
    ui->emptyFolderPage->setAcceptDrops(true);
    mProxyModel->setSourceModel(mModel.get());

    connect(mProxyModel.get(),
            &NodeSelectorProxyModel::modelSorted,
            this,
            &NodeSelectorTreeViewWidget::onExpandReady);
    connect(mProxyModel.get(),
            &NodeSelectorProxyModel::modelSorted,
            this,
            &NodeSelectorTreeViewWidget::viewReady);
    connect(mProxyModel.get(),
            &NodeSelectorProxyModel::modelSorted,
            this,
            &NodeSelectorTreeViewWidget::setViewPage);
    connect(mModel.get(),
            &QAbstractItemModel::rowsInserted,
            this,
            &NodeSelectorTreeViewWidget::checkViewOnModelChange);
    connect(mModel.get(),
            &QAbstractItemModel::rowsRemoved,
            this,
            &NodeSelectorTreeViewWidget::checkViewOnModelChange);
    connect(mModel.get(),
            &NodeSelectorModel::blockUi,
            this,
            &NodeSelectorTreeViewWidget::setLoadingSceneVisible);
    connect(mModel.get(),
            &NodeSelectorModel::modelModified,
            this,
            &NodeSelectorTreeViewWidget::onModelModified);
    connect(mModel.get(),
            &NodeSelectorModel::itemsMoved,
            this,
            &NodeSelectorTreeViewWidget::onItemsMoved);
    connect(mModel.get(),
            &NodeSelectorModel::nodesAdded,
            this,
            &NodeSelectorTreeViewWidget::onNodesAdded);

#ifdef __APPLE__
    ui->tMegaFolders->setAnimated(false);
#endif

    connect(&mNodesUpdateTimer,
            &QTimer::timeout,
            this,
            &NodeSelectorTreeViewWidget::processCachedNodesUpdated);
    mNodesUpdateTimer.start(CHECK_UPDATED_NODES_INTERVAL);
}

void NodeSelectorTreeViewWidget::initEmptyMessages()
{
    auto emptyLabelInfo(getEmptyLabel());
    ui->descriptionEmptyLabel->hide();
    ui->titleEmptyLabel->hide();
    if (!emptyLabelInfo.description.isEmpty())
    {
        ui->descriptionEmptyLabel->show();
        ui->descriptionEmptyLabel->setText(emptyLabelInfo.description);
    }
    if (!emptyLabelInfo.title.isEmpty())
    {
        ui->titleEmptyLabel->show();
        ui->titleEmptyLabel->setText(emptyLabelInfo.title);
    }
}

bool NodeSelectorTreeViewWidget::event(QEvent* event)
{
    if (event->type() == QEvent::LanguageChange)
    {
        if (!ui->tMegaFolders->rootIndex().isValid())
        {
            updateRootTitle();
        }
        ui->retranslateUi(this);

        if (mSelectType)
        {
            mSelectType->updateCustomButtonsText(this);
        }

        initEmptyMessages();
    }
    else if (event->type() == QEvent::MouseButtonRelease)
    {
        clearSelection();
    }

    return QWidget::event(event);
}

bool NodeSelectorTreeViewWidget::eventFilter(QObject* watched, QEvent* event)
{
    if (event->type() == QEvent::DragEnter)
    {
        if (auto dropEvent = static_cast<QDragEnterEvent*>(event))
        {
            if (!dropEvent->mimeData()->urls().isEmpty())
            {
                ui->tMegaFolders->dragEnterEvent(dropEvent);
            }
            else if (mModel->canDropMimeData(dropEvent->mimeData(),
                                             Qt::MoveAction,
                                             -1,
                                             -1,
                                             mModel->index(0, 0, QModelIndex())))
            {
                dropEvent->acceptProposedAction();
            }
        }
    }
    else if (event->type() == QEvent::DragMove)
    {
        if (auto dropEvent = static_cast<QDragEnterEvent*>(event))
        {
            if (!dropEvent->mimeData()->urls().isEmpty())
            {
                ui->tMegaFolders->dragMoveEvent(dropEvent);
            }
        }
    }
    else if (event->type() == QEvent::Drop)
    {
        if (auto dropEvent = static_cast<QDragEnterEvent*>(event))
        {
            if (!dropEvent->mimeData()->urls().isEmpty())
            {
                ui->tMegaFolders->dropEvent(dropEvent);
            }
        }
    }
    else if (event->type() == QEvent::Resize)
    {
        if (watched == ui->tMegaFolders->viewport())
        {
            updateColumnsWidth(false);
        }
    }
    else if (watched == ui->emptyFolderPage && event->type() == QEvent::ContextMenu)
    {
        ui->tMegaFolders->contextMenuEvent(static_cast<QContextMenuEvent*>(event));
    }

    return QWidget::eventFilter(watched, event);
}

void NodeSelectorTreeViewWidget::setTitleText(const QString& nodeName)
{
    ui->lFolderName->setText(nodeName);
}

void NodeSelectorTreeViewWidget::clearSelection()
{
    ui->tMegaFolders->clearSelection();
}

void NodeSelectorTreeViewWidget::abort()
{
    mModel->abort();
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

NodeSelectorProxyModel* NodeSelectorTreeViewWidget::getProxyModel()
{
    return mProxyModel.get();
}

bool NodeSelectorTreeViewWidget::isInRootView() const
{
    return !ui->tMegaFolders->rootIndex().isValid();
}

bool NodeSelectorTreeViewWidget::isEmpty() const
{
    return ui->tMegaFolders->model()->rowCount(QModelIndex()) == 0;
}

void NodeSelectorTreeViewWidget::updateLoadingMessage(std::shared_ptr<MessageInfo> message)
{
    ui->tMegaFolders->getLoadingMessageHandler()->updateMessage(message);
}

void NodeSelectorTreeViewWidget::enableDragAndDrop(bool enable)
{
    ui->tMegaFolders->setDragEnabled(enable);
    ui->tMegaFolders->viewport()->setAcceptDrops(enable);
    ui->tMegaFolders->setDropIndicatorShown(enable);
    ui->tMegaFolders->setDragDropMode(enable ? QAbstractItemView::DragDrop :
                                               QAbstractItemView::NoDragDrop);
}

void NodeSelectorTreeViewWidget::setTitle(const QString& title)
{
    setTitleText(title);
}

void NodeSelectorTreeViewWidget::mousePressEvent(QMouseEvent* event)
{
    if (event->button() == Qt::BackButton && ui->bBack->isEnabled())
    {
        onGoBackClicked();
    }
    else if (event->button() == Qt::ForwardButton && ui->bForward->isEnabled())
    {
        onGoForwardClicked();
    }
}

void NodeSelectorTreeViewWidget::onRootIndexChanged(const QModelIndex&)
{
    updateColumnsWidth(true);
}

void NodeSelectorTreeViewWidget::updateColumnsWidth(bool updateVisibleColumnCounter)
{
    if (updateVisibleColumnCounter)
    {
        mVisibleColumns.clear();

        for (int column = 0; column < ui->tMegaFolders->header()->count(); ++column)
        {
            if (!ui->tMegaFolders->header()->isSectionHidden(column))
            {
                mVisibleColumns.append(column);
            }
        }
    }

    if (!mVisibleColumns.isEmpty() && !mManuallyResizedColumn)
    {
        int widthTotal(0);
        int minWidth(100);
        int maxSecondaryColumnWidth(200);
        double secondaryColumnProportion(0.2);

        for (QList<int>::const_reverse_iterator column = mVisibleColumns.crbegin();
             column != mVisibleColumns.crend();
             ++column)
        {
            int width(0);

            if ((*column) == NodeSelectorModel::COLUMN::NODE)
            {
                // Total minus the rest of columns
                width = std::max(ui->tMegaFolders->viewport()->width() - widthTotal, minWidth * 2);
            }
            else
            {
                width =
                    std::max(std::min(qRound(ui->tMegaFolders->width() * secondaryColumnProportion),
                                      maxSecondaryColumnWidth),
                             minWidth);
                widthTotal += width;
            }

            ui->tMegaFolders->setColumnWidth((*column), width);
        }
    }
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

void NodeSelectorTreeViewWidget::checkViewOnModelChange()
{
    checkBackForwardButtons();
    setViewPage();

    setEmptyFolderPage();
}

void NodeSelectorTreeViewWidget::checkNewFolderAdded(QPointer<NodeSelectorModelItem> item)
{
    if (mNewFolderAdded)
    {
        // If the row inserted is the new row, stop iterating over the new insertions
        if (item->getNode()->getHandle() == mNewFolderHandle)
        {
            auto newFolderIndex(mProxyModel->getIndexFromHandle(mNewFolderHandle));

            onItemDoubleClick(newFolderIndex);
            selectionHasChanged(QModelIndexList() << newFolderIndex);

            mNewFolderHandle = mega::INVALID_HANDLE;
            mNewFolderAdded = false;
        }
    }
}

void NodeSelectorTreeViewWidget::onExpandReady()
{
    if (ui->tMegaFolders->model() == nullptr)
    {
        ui->tMegaFolders->setContextMenuPolicy(Qt::DefaultContextMenu);
        ui->tMegaFolders->setExpandsOnDoubleClick(false);
        ui->tMegaFolders->header()->setDefaultAlignment(Qt::AlignLeft);
        ui->tMegaFolders->header()->setDefaultSectionSize(35);
        ui->tMegaFolders->setItemDelegate(new NodeRowDelegate(ui->tMegaFolders));
        ui->tMegaFolders->setTextElideMode(Qt::ElideMiddle);

        ui->tMegaFolders->sortByColumn(NodeSelectorModel::NODE, Qt::AscendingOrder);
        ui->tMegaFolders->setModel(mProxyModel.get());

        ui->tMegaFolders->header()->setVisible(true);
        ui->tMegaFolders->header()->setProperty("HeaderIconCenter", true);

        // those connects needs to be done after the model is set, do not move them

        connect(ui->tMegaFolders->selectionModel(),
                &QItemSelectionModel::selectionChanged,
                this,
                &NodeSelectorTreeViewWidget::onSelectionChanged);
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
                &NodeSelectorTreeViewWidget::onbNewFolderClicked);
        connect(ui->tMegaFolders,
                &NodeSelectorTreeView::getMegaLinkClicked,
                this,
                &NodeSelectorTreeViewWidget::onGenMEGALinkClicked);
        connect(ui->tMegaFolders,
                &QTreeView::doubleClicked,
                this,
                &NodeSelectorTreeViewWidget::onItemDoubleClick);
        connect(ui->tMegaFolders,
                &NodeSelectorTreeView::nodeSelected,
                this,
                &NodeSelectorTreeViewWidget::okBtnClicked);
        connect(ui->bForward,
                &QPushButton::clicked,
                this,
                &NodeSelectorTreeViewWidget::onGoForwardClicked);
        connect(ui->bBack,
                &QPushButton::clicked,
                this,
                &NodeSelectorTreeViewWidget::onGoBackClicked);
        connect(ui->tMegaFolders->header(),
                &QHeaderView::sectionResized,
                this,
                &NodeSelectorTreeViewWidget::onSectionResized);

        mSelectType->makeViewCustomConnections(ui->tMegaFolders, this);
        makeCustomConnections();

        setRootIndex(mModel->hasTopRootIndex() ? mProxyModel->index(0, 0) : QModelIndex());

        setStyleSheet(styleSheet());
    }
}

void NodeSelectorTreeViewWidget::onGoBackClicked()
{
    auto rootIndex(ui->tMegaFolders->rootIndex());
    auto rootIndexHandle(getHandleByIndex(rootIndex));
    if (rootIndexHandle != mega::INVALID_HANDLE)
    {
        mNavigationInfo.appendToForward(rootIndexHandle);
    }
    QModelIndex indexToGo = getIndexFromHandle(mNavigationInfo.backwardHandles.last());

    setRootIndex(indexToGo);
    checkBackForwardButtons();

    if (rootIndex.isValid())
    {
        selectIndex(rootIndex, true);
    }
}

void NodeSelectorTreeViewWidget::onRemoveIndexFromGoBack(const QModelIndex& indexToRemove)
{
    if (indexToRemove.isValid())
    {
        auto changeRootIndex = [this](QModelIndex removedIndex)
        {
            auto parentIndex(removedIndex.parent());

            // Avoid adding the cloud drive
            if (parentIndex.parent().isValid())
            {
                setRootIndex(parentIndex);
            }
            else
            {
                setRootIndex(mModel->hasTopRootIndex() ? mProxyModel->index(0, 0) : QModelIndex());
                mNavigationInfo.backwardHandles.clear();
            }
        };

        if (indexToRemove == ui->tMegaFolders->rootIndex())
        {
            changeRootIndex(indexToRemove);
        }
        else
        {
            // If the index is in the list of backward handles
            // set the parent as root index and remove the parent from the list of backward handles
            auto indexHandleToRemove(getHandleByIndex(indexToRemove));
            auto handlePos(mNavigationInfo.backwardHandles.indexOf(indexHandleToRemove));
            if (handlePos >= 0)
            {
                changeRootIndex(indexToRemove);
            }
        }
    }
}

void NodeSelectorTreeViewWidget::onGoForwardClicked()
{
    mNavigationInfo.appendToBackward(getHandleByIndex(ui->tMegaFolders->rootIndex()));
    QModelIndex indexToGo = getIndexFromHandle(mNavigationInfo.forwardHandles.last());
    mNavigationInfo.forwardHandles.removeLast();
    setRootIndex(indexToGo);
    checkBackForwardButtons();

    selectionHasChanged(ui->tMegaFolders->selectedRows());
}

MegaHandle NodeSelectorTreeViewWidget::getHandleByIndex(const QModelIndex& idx)
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

void NodeSelectorTreeViewWidget::onbNewFolderClicked()
{
    auto parentNode = mProxyModel->getNode(ui->tMegaFolders->rootIndex());
    if (!parentNode)
    {
        parentNode = MegaSyncApp->getRootNode();
        if (!parentNode)
            return;
    }

    QPointer<NewFolderDialog> dialog(new NewFolderDialog(parentNode, ui->tMegaFolders));
    dialog->init();
    DialogOpener::showDialog(dialog,
                             [this, dialog]()
                             {
                                 auto newNode = dialog->getNewNode();
                                 // IF the dialog return a node, there are two scenarios:
                                 // 1) The dialog has been accepted, a new folder has been created
                                 // 2) The dialog has been rejected because the folder already
                                 // exists. If so, select the existing folder
                                 if (newNode)
                                 {
                                     mNewFolderHandle = newNode->getHandle();
                                     mNewFolderAdded = true;
#ifdef Q_OS_LINUX
                                     // It seems that the NodeSelector is not activated when the
                                     // NewFolderDialog is closed, so the ui->tMegaFolders is not
                                     // correctly focused
                                     qApp->setActiveWindow(parentWidget()->parentWidget());
#endif

                                     // Set the focus to the view to allow the user to press enter
                                     // (or go back, in a future feature)
                                     ui->tMegaFolders->setFocus();
                                 }
                             });
}

bool NodeSelectorTreeViewWidget::isAllowedToEnterInIndex(const QModelIndex& idx)
{
    return mSelectType->isAllowedToNavigateInside(idx);
}

void NodeSelectorTreeViewWidget::onItemDoubleClick(const QModelIndex& index)
{
    if (!isAllowedToEnterInIndex(index))
    {
        auto item = mModel->getItemByIndex(index);
        if (item && item->getNode()->isFile())
        {
            MegaSyncApp->downloadACtionClickedWithHandles(QList<mega::MegaHandle>()
                                                          << item->getNode()->getHandle());
        }
        return;
    }

    mNavigationInfo.appendToBackward(getHandleByIndex(ui->tMegaFolders->rootIndex()));
    mNavigationInfo.removeFromForward(mProxyModel->getHandle(index));

    setRootIndex(index);
    checkBackForwardButtons();
}

void NodeSelectorTreeViewWidget::checkButtonsVisibility()
{
    mSelectType->newFolderButtonVisibility(this);
}

void NodeSelectorTreeViewWidget::checkOkCancelButtonsVisibility()
{
    mSelectType->okCancelButtonsVisibility(this);
}

void NodeSelectorTreeViewWidget::addCustomButtons(NodeSelectorTreeViewWidget* wdg)
{
    auto buttonsMap = mSelectType->addCustomButtons(wdg);
    foreach(auto& id, buttonsMap.keys())
    {
        auto button = buttonsMap.value(id);
        if (button)
        {
            ui->customButtonsLayout->insertWidget(0, button);
            connect(button,
                    &QPushButton::clicked,
                    this,
                    [this, id]()
                    {
                        emit onCustomButtonClicked(id);
                    });
        }
    }
}

std::unique_ptr<NodeSelectorProxyModel> NodeSelectorTreeViewWidget::createProxyModel()
{
    return std::unique_ptr<NodeSelectorProxyModel>(new NodeSelectorProxyModel);
}

void NodeSelectorTreeViewWidget::setLoadingSceneVisible(bool blockUi)
{
    if (mSelectType->footerVisible())
    {
        ui->footer->setVisible(!blockUi);
    }
    ui->tMegaFolders->loadingView().toggleLoadingScene(blockUi);

    if (!blockUi)
    {
        expandPendingIndexes();
        selectPendingIndexes();
    }
}

void NodeSelectorTreeViewWidget::setViewPage()
{
    if (mModel)
    {
        auto topRootIndex = mModel->hasTopRootIndex() ? mModel->index(0, 0) : QModelIndex();

        if (mModel->rowCount(topRootIndex) == 0 && showEmptyView())
        {
            ui->stackedWidget->setCurrentWidget(ui->emptyPage);
            return;
        }
    }
    ui->stackedWidget->setCurrentWidget(ui->treeViewPage);
}

QModelIndex NodeSelectorTreeViewWidget::getAddedNodeParent(mega::MegaHandle parentHandle)
{
    return mModel->findIndexByNodeHandle(parentHandle, QModelIndex());
}

void NodeSelectorTreeViewWidget::onUiBlocked(bool state)
{
    if (mUiBlocked != state)
    {
        mUiBlocked = state;
    }

    ui->bNewFolder->setDisabled(state);
    ui->bCancel->setDisabled(state);
    ui->searchButtonsWidget->setDisabled(state);

    if (!state)
    {
        selectionHasChanged(ui->tMegaFolders->selectedRows());
        checkBackForwardButtons();
    }
    else
    {
        ui->bBack->setEnabled(false);
        ui->bForward->setEnabled(false);
        ui->bOk->setDisabled(true);
    }
}

void NodeSelectorTreeViewWidget::onSelectionChanged(const QItemSelection& selected,
                                                    const QItemSelection& deselected)
{
    Q_UNUSED(deselected)
    Q_UNUSED(selected)

    if (!mUiBlocked)
    {
        selectionHasChanged(ui->tMegaFolders->selectedRows());
    }
}

void NodeSelectorTreeViewWidget::onModelModified()
{
    mSelectType->selectionHasChanged(this);
}

void NodeSelectorTreeViewWidget::selectionHasChanged(const QModelIndexList& selected)
{
    ui->bOk->setEnabled(mSelectType->okButtonEnabled(this, selected));
    mSelectType->selectionHasChanged(this);
}

void NodeSelectorTreeViewWidget::onRenameClicked()
{
    auto node = std::unique_ptr<MegaNode>(mMegaApi->getNodeByHandle(getSelectedNodeHandle()));
    int access = mMegaApi->getAccess(node.get());
    // This is for an extra protection as we don´t show the rename action if one of this conditions
    // are not met
    if (!node || access < MegaShare::ACCESS_FULL || !node->isNodeKeyDecrypted())
    {
        return;
    }

    QPointer<RenameRemoteNodeDialog> dialog(new RenameRemoteNodeDialog(std::move(node), this));
    dialog->init();
    DialogOpener::showDialog(dialog);
}

void NodeSelectorTreeViewWidget::onDeleteClicked(const QList<mega::MegaHandle>& handles,
                                                 bool permanently,
                                                 bool showConfirmationMessageBox)
{
    if (handles.isEmpty())
    {
        return;
    }

    auto getNode = [this](mega::MegaHandle handle) -> std::shared_ptr<mega::MegaNode>
    {
        auto node = std::shared_ptr<MegaNode>(mMegaApi->getNodeByHandle(handle));

        // This is for an extra protection as we don´t show the rename action if oxne of this
        // conditions are not met
        if (!node || !node->isNodeKeyDecrypted())
        {
            return nullptr;
        }

        return node;
    };

    if (showConfirmationMessageBox)
    {
        MessageDialogInfo msgInfo;
        msgInfo.parent = ui->tMegaFolders;
        msgInfo.buttons = QMessageBox::Yes | QMessageBox::No;
        msgInfo.defaultButton = QMessageBox::Yes;
        msgInfo.finishFunc = [this, handles, permanently](QPointer<MessageDialogResult> msg)
        {
            if (msg->result() == QMessageBox::Yes)
            {
                mModel->deleteNodes(handles, permanently);
            }
        };

        if (permanently)
        {
            msgInfo.descriptionText = tr("You cannot undo this action");
        }
        else
        {
            msgInfo.descriptionText = tr(
                "Any shared files or folders will no longer be accessible to the people you shared "
                "them with. You can still access these items in the Rubbish bin, restore, and "
                "share "
                "them.");
        }

        auto type(Utilities::getHandlesType(handles));

        if (permanently)
        {
            msgInfo.buttonsText.insert(QMessageBox::Yes, tr("Delete"));
            msgInfo.buttonsText.insert(QMessageBox::No, tr("Cancel"));

            if (type == Utilities::HandlesType::FILES)
            {
                msgInfo.titleText =
                    tr("You are about to permanently delete %n file. Would you like to proceed?",
                       "",
                       handles.size());
            }
            else if (type == Utilities::HandlesType::FOLDERS)
            {
                msgInfo.titleText =
                    tr("You are about to permanently delete %n folder. Would you like to proceed?",
                       "",
                       handles.size());
            }
            else
            {
                msgInfo.titleText =
                    tr("You are about to permanently delete %n items. Would you like to proceed?",
                       "",
                       handles.size());
            }
        }
        else
        {
            msgInfo.buttonsText.insert(QMessageBox::Yes, tr("Move"));
            msgInfo.buttonsText.insert(QMessageBox::No, tr("Don’t move"));

            auto node = getNode(handles.first());
            if (handles.size() == 1 && node)
            {
                msgInfo.titleText =
                    tr("Move %1 to Rubbish bin?").arg(MegaNodeNames::getNodeName(node.get()));
            }
            else
            {
                msgInfo.titleText = tr("Move %n items to Rubbish bin?", "", handles.size());
            }
        }

        MessageDialogOpener::warning(msgInfo);
    }
    else
    {
        mModel->deleteNodes(handles, permanently);
    }
}

void NodeSelectorTreeViewWidget::onLeaveShareClicked(const QList<mega::MegaHandle>& handles)
{
    if (handles.isEmpty())
    {
        return;
    }

    MessageDialogInfo msgInfo;
    msgInfo.parent = ui->tMegaFolders;
    msgInfo.buttons = QMessageBox::Yes | QMessageBox::No;
    msgInfo.defaultButton = QMessageBox::Yes;
    msgInfo.buttonsText.insert(QMessageBox::Yes, tr("Leave"));
    msgInfo.buttonsText.insert(QMessageBox::No, tr("Don’t leave"));
    msgInfo.titleText = tr("Leave this shared folder?", "", handles.size());
    msgInfo.descriptionText =
        tr("If you leave the folder, you will not be able to see it again.", "", handles.size());
    msgInfo.finishFunc = [this, handles](QPointer<MessageDialogResult> msg)
    {
        if (msg->result() == QMessageBox::Yes)
        {
            mModel->deleteNodes(handles, true);
        }
    };
    MessageDialogOpener::warning(msgInfo);
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
    if (!nodes)
    {
        return false;
    }

    for (int i = 0; i < nodes->size(); i++)
    {
        MegaNode* node = nodes->get(i);

        if (mModel->rootNodeUpdated(node))
        {
            continue;
        }

        if (node->getParentHandle() != mega::INVALID_HANDLE)
        {
            if (node->getChanges() & MegaNode::CHANGE_TYPE_REMOVED &&
                (!mMergeTargetFolders.isEmpty() && mMergeTargetFolders.contains(node->getHandle())))
            {
                mMergeSourceFolderRemoved.append(UpdateNodesInfo(node, QModelIndex()));
            }

            auto index(mModel->findIndexByNodeHandle(node->getHandle(), QModelIndex()));
            auto existenceType(getNodeOnModelState(index, node));

            if (existenceType == NodeState::DOESNT_EXIST)
            {
                continue;
            }

            if (node->getChanges() & (MegaNode::CHANGE_TYPE_PARENT | MegaNode::CHANGE_TYPE_NEW))
            {
                std::unique_ptr<mega::MegaNode> parentNode(
                    MegaSyncApp->getMegaApi()->getNodeByHandle(node->getParentHandle()));

                if (existenceType == NodeState::REMOVE)
                {
                    mRemovedNodes.append(UpdateNodesInfo(node, index));
                }
                else
                {
                    std::unique_ptr<mega::MegaNode> parentNode(
                        MegaSyncApp->getMegaApi()->getNodeByHandle(node->getParentHandle()));
                    if (parentNode)
                    {
                        // Check if the node exists or if we need to add it
                        if (existenceType == NodeState::ADD || existenceType == NodeState::MOVED)
                        {
                            if (!node->isFile() || mModel->showFiles())
                            {
                                if (mUpdatedNodesBeforeAdded.contains(node->getHandle()))
                                {
                                    mUpdatedNodesBeforeAdded.remove(node->getHandle());
                                }

                                mAddedNodesByParentHandle.insert(node->getParentHandle(),
                                                                 UpdateNodesInfo(node, index));
                            }

                            if (existenceType == NodeState::MOVED)
                            {
                                mRemoveMovedNodes.append(UpdateNodesInfo(node, index));
                            }
                        }
                        else if (existenceType == NodeState::EXISTS_BUT_OUT_OF_VIEW &&
                                 mParentOfRestoredNodes.contains(node->getParentHandle()))
                        {
                            mUpdatedButInvisibleNodes.append(UpdateNodesInfo(node, index));
                        }
                        else if (existenceType == NodeState::EXISTS_BUT_PARENT_UNINITIALISED ||
                                 existenceType == NodeState::MOVED_OUT_OF_VIEW)
                        {
                            if (existenceType == NodeState::MOVED_OUT_OF_VIEW)
                            {
                                mRemoveMovedNodes.append(UpdateNodesInfo(node, index));
                            }

                            if (mMergeTargetFolders.isEmpty() ||
                                mMergeTargetFolders.key(node->getParentHandle(),
                                                        mega::INVALID_HANDLE) ==
                                    mega::INVALID_HANDLE)
                            {
                                mUpdatedButInvisibleNodes.append(UpdateNodesInfo(node, index));
                            }
                        }
                    }
                }
            }
            else if (node->getChanges() & MegaNode::CHANGE_TYPE_NAME)
            {
                if (existenceType == NodeState::EXISTS)
                {
                    mRenamedNodesByHandle.append(UpdateNodesInfo(node, index));
                }
            }
            // Moved or new version added
            else if (node->getChanges() & MegaNode::CHANGE_TYPE_REMOVED)
            {
                if (existenceType == NodeState::EXISTS)
                {
                    mRemovedNodes.append(UpdateNodesInfo(node, index));
                }
                else if (existenceType == NodeState::EXISTS_BUT_PARENT_UNINITIALISED)
                {
                    mUpdatedButInvisibleNodes.append(UpdateNodesInfo(node, index));
                }
            }
            else if (node->getChanges() & MegaNode::CHANGE_TYPE_ATTRIBUTES)
            {
                if (existenceType == NodeState::EXISTS)
                {
                    mUpdatedNodes.append(UpdateNodesInfo(node, index));
                }
                else if (existenceType == NodeState::ADD)
                {
                    mUpdatedNodesBeforeAdded.insert(node->getHandle(),
                                                    UpdateNodesInfo(node, index));
                }
            }
        }
    }

    if (areThereNodesToUpdate())
    {
        if (shouldUpdateImmediately())
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
    else
    {
        return false;
    }
}

bool NodeSelectorTreeViewWidget::shouldUpdateImmediately()
{
    int totalSize = mUpdatedNodes.size();
    if (totalSize > IMMEDIATE_CHECK_UPDATES_NODES_THRESHOLD)
    {
        return true;
    }
    totalSize += mRemovedNodes.size();
    if (totalSize > IMMEDIATE_CHECK_UPDATES_NODES_THRESHOLD)
    {
        return true;
    }
    totalSize += mRemoveMovedNodes.size();
    if (totalSize > IMMEDIATE_CHECK_UPDATES_NODES_THRESHOLD)
    {
        return true;
    }
    totalSize += mRenamedNodesByHandle.size();
    if (totalSize > IMMEDIATE_CHECK_UPDATES_NODES_THRESHOLD)
    {
        return true;
    }
    totalSize += mAddedNodesByParentHandle.size();
    if (totalSize > IMMEDIATE_CHECK_UPDATES_NODES_THRESHOLD)
    {
        return true;
    }
    totalSize += mUpdatedButInvisibleNodes.size();
    if (totalSize > IMMEDIATE_CHECK_UPDATES_NODES_THRESHOLD)
    {
        return true;
    }
    totalSize += mMergeSourceFolderRemoved.size();
    if (totalSize > IMMEDIATE_CHECK_UPDATES_NODES_THRESHOLD)
    {
        return true;
    }
    return false;
}

bool NodeSelectorTreeViewWidget::areThereNodesToUpdate()
{
    return !mUpdatedNodes.isEmpty() || !mRemovedNodes.isEmpty() ||
           !mRenamedNodesByHandle.isEmpty() || !mAddedNodesByParentHandle.isEmpty() ||
           !mRemoveMovedNodes.isEmpty() || !mUpdatedButInvisibleNodes.isEmpty() ||
           !mMergeSourceFolderRemoved.isEmpty();
}

void NodeSelectorTreeViewWidget::updateRootTitle()
{
    setTitleText(getRootText());
}

void NodeSelectorTreeViewWidget::expandPendingIndexes()
{
    auto indexesToBeExpanded = mModel->needsToBeExpanded();
    if (!indexesToBeExpanded.isEmpty())
    {
        foreach(auto item, indexesToBeExpanded)
        {
            QModelIndex proxyIndex;
            auto handle(item.first);

            if (handle != mega::INVALID_HANDLE)
            {
                proxyIndex = mProxyModel->getIndexFromHandle(handle);
            }

            if (proxyIndex.isValid())
            {
                ui->tMegaFolders->setExpanded(proxyIndex, true);
            }
        }
    }
}

void NodeSelectorTreeViewWidget::selectPendingIndexes()
{
    auto indexesToBeSelected = mModel->needsToBeSelected();
    if (!indexesToBeSelected.isEmpty())
    {
        // Disconnect the signal to check the state when finished
        disconnect(ui->tMegaFolders->selectionModel(),
                   &QItemSelectionModel::selectionChanged,
                   this,
                   &NodeSelectorTreeViewWidget::onSelectionChanged);

        bool allSelected(true);
        foreach(auto item, indexesToBeSelected)
        {
            QModelIndex proxyIndex;
            auto handle(item.first);

            if (handle != mega::INVALID_HANDLE)
            {
                proxyIndex = mProxyModel->getIndexFromHandle(handle);

                if (proxyIndex.isValid())
                {
                    selectIndex(proxyIndex, true, false);
                }
                else
                {
                    setSelectedNodeHandle(handle);
                    allSelected = false;
                }
            }
        }
        // Connect it again
        connect(ui->tMegaFolders->selectionModel(),
                &QItemSelectionModel::selectionChanged,
                this,
                &NodeSelectorTreeViewWidget::onSelectionChanged);

        if (allSelected)
        {
            onSelectionChanged(QItemSelection(), QItemSelection());
        }
    }
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

    if (setCurrent)
    {
        ui->tMegaFolders->selectionModel()->setCurrentIndex(index,
                                                            selectionFlag |
                                                                QItemSelectionModel::Rows);
    }
    ui->tMegaFolders->selectionModel()->select(index, selectionFlag | QItemSelectionModel::Rows);
    ui->tMegaFolders->scrollTo(index, QAbstractItemView::ScrollHint::PositionAtCenter);
}

bool NodeSelectorTreeViewWidget::increaseMovingNodes(int number)
{
    resetMoveNodesToSelect();
    return mModel->increaseMovingNodes(number);
}

bool NodeSelectorTreeViewWidget::decreaseMovingNodes(int number)
{
    return mModel->moveProcessedByNumber(number);
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

void NodeSelectorTreeViewWidget::onItemsMoved()
{
    if (!mMovedHandlesToSelect.isEmpty() || !mMergeTargetFolders.isEmpty())
    {
        clearSelection();
    }

    if (!mMovedHandlesToSelect.isEmpty())
    {
        mModel->selectIndexesByHandleAsync(mMovedHandlesToSelect);
    }

    if (!mMergeTargetFolders.isEmpty())
    {
        mModel->selectIndexesByHandleAsync(mMergeTargetFolders.values());
    }

    mMovedHandlesToSelect.clear();
    mParentOfRestoredNodes.clear();
    mMergeTargetFolders.clear();
}

void NodeSelectorTreeViewWidget::resetMoveNodesToSelect()
{
    // Reset selection system
    if (mModel->getMoveRequestsCounter() == 0)
    {
        // Reset selection system
        mMovedHandlesToSelect.clear();
    }
}

void NodeSelectorTreeViewWidget::onNodesAdded(
    const QList<QPointer<NodeSelectorModelItem>>& itemsAdded)
{
    // If we are moving nodes, the loading view is visible
    if (mModel->isMovingNodes())
    {
        auto moveProcessCounter(0);

        for (const auto& item: itemsAdded)
        {
            if (mMergeTargetFolders.isEmpty() ||
                mMergeTargetFolders.key(item->getNode()->getParentHandle(), mega::INVALID_HANDLE) ==
                    mega::INVALID_HANDLE)
            {
                mMovedHandlesToSelect.insert(item->getNode()->getHandle());
                moveProcessCounter++;
            }
        }

        mModel->moveProcessedByNumber(moveProcessCounter);
    }
    // Creating a new folder using the "New folder" button never happens while moving nodes
    else
    {
        for (const auto& item: itemsAdded)
        {
            checkNewFolderAdded(item);
        }
    }
}

void NodeSelectorTreeViewWidget::removeItemByHandle(mega::MegaHandle handle)
{
    auto index = mModel->findIndexByNodeHandle(handle, QModelIndex());
    if (index.isValid())
    {
        auto proxyIndex(mProxyModel->mapFromSource(index));
        if (proxyIndex.isValid())
        {
            // In case one of the selected indexes has been also removed
            mMovedHandlesToSelect.remove(handle);

            onRemoveIndexFromGoBack(proxyIndex);

            if (mNavigationInfo.forwardHandles.contains(handle))
            {
                mNavigationInfo.forwardHandles.removeLast();
            }

            checkBackForwardButtons();

            mProxyModel->deleteNode(proxyIndex);
            mNavigationInfo.remove(handle);
        }
    }
}

void NodeSelectorTreeViewWidget::processCachedNodesUpdated()
{
    // We check if the model is being modified (insert rows, remove rows...etc) before each action
    // in order to avoid calling twice to begininsertrows (as some of these actions are performed in
    // different threads...)
    if (!mProxyModel->isModelProcessing() && !mModel->isRequestingNodes() &&
        areThereNodesToUpdate())
    {
        int moveProcessedCounter(0);

        if (!mModel->isBeingModified())
        {
            for (const auto& info: std::as_const(mRenamedNodesByHandle))
            {
                updateNode(info, true);
            }
            mRenamedNodesByHandle.clear();
        }

        if (!mModel->isBeingModified())
        {
            for (const auto& info: std::as_const(mUpdatedNodes))
            {
                updateNode(info, false);
            }
            mUpdatedNodes.clear();
        }

        if (!mModel->isBeingModified())
        {
            for (const auto& info: std::as_const(mRemovedNodes))
            {
                removeItemByHandle(info.handle);

                if (!mNodesToBeReplaced.remove(info.handle))
                {
                    moveProcessedCounter++;
                }
            }
            mRemovedNodes.clear();
        }

        if (!mModel->isBeingModified())
        {
            for (const auto& info: std::as_const(mRemoveMovedNodes))
            {
                removeItemByHandle(info.handle);
            }
            mRemoveMovedNodes.clear();
        }

        if (!mModel->isBeingModified() && !mUpdatedButInvisibleNodes.isEmpty())
        {
            for (const auto& info: std::as_const(mUpdatedButInvisibleNodes))
            {
                if (info.handle != mega::INVALID_HANDLE)
                {
                    // Just in case
                    if (info.node->getChanges() == mega::MegaNode::CHANGE_TYPE_REMOVED)
                    {
                        removeItemByHandle(info.handle);
                    }
                    else
                    {
                        mMovedHandlesToSelect.insert(info.handle);
                    }
                    moveProcessedCounter++;
                }
            }

            mUpdatedButInvisibleNodes.clear();
        }

        if (!mModel->isBeingModified() && !mMergeSourceFolderRemoved.isEmpty())
        {
            for (const auto& info: std::as_const(mMergeSourceFolderRemoved))
            {
                if (info.handle != mega::INVALID_HANDLE)
                {
                    moveProcessedCounter++;
                }
            }

            mMergeSourceFolderRemoved.clear();
        }

        if (!mModel->isBeingModified())
        {
            foreach(auto& parentHandle, mAddedNodesByParentHandle.uniqueKeys())
            {
                auto parentIndex = getAddedNodeParent(parentHandle);
                auto infos(mAddedNodesByParentHandle.values(parentHandle));
                QList<std::shared_ptr<mega::MegaNode>> addedNodes;

                for (const auto& info: infos)
                {
                    auto handle(info.handle);

                    if (mUpdatedNodesBeforeAdded.contains(handle))
                    {
                        addedNodes.append(mUpdatedNodesBeforeAdded.take(handle).node);
                    }
                    else
                    {
                        addedNodes.append(info.node);
                    }
                }

                if (!mModel->addNodes(addedNodes, parentIndex))
                {
                    mModel->moveProcessedByNumber(addedNodes.size());
                }

                // Only for root indexes
                auto proxyParentIndex(mProxyModel->mapFromSource(parentIndex));
                if (!proxyParentIndex.parent().isValid())
                {
                    ui->tMegaFolders->setExpanded(proxyParentIndex, true);
                }
            }

            mAddedNodesByParentHandle.clear();
            mUpdatedNodesBeforeAdded.clear();
        }

        mModel->moveProcessedByNumber(moveProcessedCounter);
    }
}

void NodeSelectorTreeViewWidget::updateNode(const UpdateNodesInfo& info, bool scrollTo)
{
    auto index = mModel->findIndexByNodeHandle(info.handle, QModelIndex());
    auto proxyIndex = mProxyModel->mapFromSource(index);

    auto isSelected(false);

    if (scrollTo)
    {
        if (ui->tMegaFolders->selectionModel())
        {
            if (proxyIndex.isValid())
            {
                isSelected = ui->tMegaFolders->selectionModel()->isSelected(proxyIndex);
            }
        }
    }

    mModel->updateItemNode(index, info.node);

    // Update proxy Index in case the node has changed the name/modified data and we are sorting by
    // any of these attributes
    proxyIndex = mProxyModel->mapFromSource(index);

    if (info.node)
    {
        if (proxyIndex.isValid() && ui->tMegaFolders->rootIndex() == proxyIndex)
        {
            setTitleText(MegaNodeNames::getNodeName(info.node.get()));
        }
    }

    if (isSelected)
    {
        // The proxy index may has changed,, update it
        proxyIndex = mProxyModel->mapFromSource(index);
        ui->tMegaFolders->scrollTo(proxyIndex, QAbstractItemView::ScrollHint::PositionAtCenter);
    }
}

void NodeSelectorTreeViewWidget::setParentOfRestoredNodes(
    const QSet<mega::MegaHandle>& parentOfRestoredNodes)
{
    mParentOfRestoredNodes = parentOfRestoredNodes;
}

void NodeSelectorTreeViewWidget::setMergeFolderHandles(
    const QMultiHash<SourceHandle, TargetHandle>& handles)
{
    mMergeTargetFolders = handles;
}

void NodeSelectorTreeViewWidget::resetMergeFolderHandles(
    const QMultiHash<SourceHandle, TargetHandle>& handles)
{
    for (auto it = handles.keyValueBegin(); it != handles.keyValueEnd(); ++it)
    {
        mMergeTargetFolders.remove(it->first);
    }
}

void NodeSelectorTreeViewWidget::setNewFolderButtonVisibility(bool state)
{
    ui->bNewFolder->setVisible(state);
    ui->tMegaFolders->setAllowNewFolderContextMenuItem(state);
}

void NodeSelectorTreeViewWidget::setSelectedNodeHandle(const MegaHandle& selectedHandle)
{
    if (selectedHandle == INVALID_HANDLE || mModel->rowCount() == 0)
    {
        return;
    }

    auto node = std::shared_ptr<MegaNode>(mMegaApi->getNodeByHandle(selectedHandle));
    if (!node)
        return;

    mProxyModel->setExpandMapped(true);

    mModel->selectIndexesByHandleAsync(QSet<mega::MegaHandle>() << node->getHandle());
    mModel->loadTreeFromNode(node);
}

MegaHandle NodeSelectorTreeViewWidget::getSelectedNodeHandle()
{
    return ui->tMegaFolders->getSelectedNodeHandle();
}

QList<MegaHandle> NodeSelectorTreeViewWidget::getMultiSelectionNodeHandle()
{
    auto selectedRows(ui->tMegaFolders->selectedRows());
    return ui->tMegaFolders->getMultiSelectionNodeHandle(selectedRows);
}

QModelIndexList NodeSelectorTreeViewWidget::getSelectedIndexes() const
{
    return ui->tMegaFolders->selectedRows();
}

void NodeSelectorTreeViewWidget::checkBackForwardButtons()
{
    ui->navigationButtons->setVisible(!mNavigationInfo.backwardHandles.isEmpty() ||
                                      !mNavigationInfo.forwardHandles.isEmpty());
    ui->bBack->setEnabled(!mNavigationInfo.backwardHandles.isEmpty());
    ui->bForward->setEnabled(!mNavigationInfo.forwardHandles.isEmpty());
}

void NodeSelectorTreeViewWidget::setRootIndex(const QModelIndex& proxy_idx)
{
    // Everytime we move among folders, we reset the selection
    ui->tMegaFolders->selectionModel()->clear();

    // In case the idx is coming from a potentially hidden column, we always take the NODE column
    // As it is the only one that have childrens
    auto node_column_idx = proxy_idx.sibling(proxy_idx.row(), NodeSelectorModel::COLUMN::NODE);

    mModel->setCurrentRootIndex(mProxyModel->mapToSource(node_column_idx));
    ui->tMegaFolders->setRootIndex(node_column_idx);
    ui->tMegaFolders->setRootIndexReadOnly(isCurrentRootIndexReadOnly());

    checkButtonsVisibility();

    // Remove in case the rootindex is in the backward list
    auto indexHandleToRemove(getHandleByIndex(node_column_idx));
    auto handlePos(mNavigationInfo.backwardHandles.indexOf(indexHandleToRemove));
    if (handlePos >= 0)
    {
        auto it = mNavigationInfo.backwardHandles.begin();
        std::advance(it, handlePos);
        mNavigationInfo.backwardHandles.erase(it, mNavigationInfo.backwardHandles.end());
    }

    onRootIndexChanged(node_column_idx);
    setEmptyFolderPage();

    if (!node_column_idx.isValid())
    {
        updateRootTitle();
        return;
    }

    auto item = NodeSelectorModel::getItemByIndex(node_column_idx);
    if (!item)
    {
        return;
    }

    auto node = item->getNode();
    if (node)
    {
        setTitleText(MegaNodeNames::getNodeName(node.get()));
    }
}

QIcon NodeSelectorTreeViewWidget::getEmptyIcon()
{
    return QIcon();
}

void NodeSelectorTreeViewWidget::setEmptyFolderPage()
{
    auto currentRootIndex(ui->tMegaFolders->rootIndex());
    auto topRootIndex(mModel->hasTopRootIndex() ? mProxyModel->index(0, 0) : QModelIndex());

    if (currentRootIndex != topRootIndex && mProxyModel->rowCount(currentRootIndex) == 0)
    {
        ui->stackedWidget->setCurrentWidget(ui->emptyFolderPage);
    }
    else
    {
        setViewPage();
    }
}

NodeSelectorTreeViewWidget::EmptyLabelInfo NodeSelectorTreeViewWidget::getEmptyLabel()
{
    return EmptyLabelInfo();
}

QModelIndex NodeSelectorTreeViewWidget::getParentIncomingShareByIndex(QModelIndex idx)
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
    MegaSyncApp->exportNodes(handles);
}

void NodeSelectorTreeViewWidget::Navigation::removeFromForward(const mega::MegaHandle& handle)
{
    if (forwardHandles.isEmpty())
        return;

    auto megaApi = MegaSyncApp->getMegaApi();
    auto p_node = std::unique_ptr<mega::MegaNode>(megaApi->getNodeByHandle(handle));

    QMap<MegaHandle, MegaHandle> parentHandles;
    while (p_node)
    {
        MegaHandle actualHandle = p_node->getHandle();
        p_node.reset(megaApi->getParentNode(p_node.get()));
        MegaHandle parentHandle = INVALID_HANDLE;
        if (p_node)
            parentHandle = p_node->getHandle();
        parentHandles.insert(parentHandle, actualHandle);
    }

    p_node.reset(megaApi->getNodeByHandle(forwardHandles.last()));
    QMap<MegaHandle, MegaHandle> actualListParentHandles;
    while (p_node)
    {
        MegaHandle actualHandle = p_node->getHandle();
        p_node.reset(megaApi->getParentNode(p_node.get()));
        MegaHandle parentHandle = INVALID_HANDLE;
        if (p_node)
            parentHandle = p_node->getHandle();
        actualListParentHandles.insert(parentHandle, actualHandle);
    }

    for (auto it = actualListParentHandles.begin(); it != actualListParentHandles.end(); ++it)
    {
        if (parentHandles.contains(it.key()))
        {
            forwardHandles.clear();
            return;
        }
    }
}

void NodeSelectorTreeViewWidget::Navigation::remove(const mega::MegaHandle& handle)
{
    backwardHandles.removeAll(handle);
    int forwardPos = forwardHandles.indexOf(handle);
    for (int i = 0; i <= forwardPos; i++)
    {
        forwardHandles.removeFirst();
    }
}

void NodeSelectorTreeViewWidget::Navigation::appendToBackward(const mega::MegaHandle& handle)
{
    if (!backwardHandles.contains(handle))
        backwardHandles.append(handle);
}

void NodeSelectorTreeViewWidget::Navigation::appendToForward(const mega::MegaHandle& handle)
{
    if (!forwardHandles.contains(handle))
        forwardHandles.append(handle);
}

void NodeSelectorTreeViewWidget::Navigation::clear()
{
    backwardHandles.clear();
    forwardHandles.clear();
}

bool SelectType::isAllowedToNavigateInside(const QModelIndex& index)
{
    auto item = NodeSelectorModel::getItemByIndex(index);
    if (!item)
    {
        return false;
    }
    return !(item->getNode()->isFile() || item->isCloudDrive() || item->isRubbishBin());
}

void SelectType::newFolderButtonVisibility(NodeSelectorTreeViewWidget* wdg)
{
    if (wdg->newFolderBtnCanBeVisisble())
    {
        wdg->setNewFolderButtonVisibility(!wdg->isEmpty() && !wdg->isCurrentRootIndexReadOnly() &&
                                          !wdg->isCurrentSelectionReadOnly());
    }
}

bool SelectType::cloudDriveIsCurrentRootIndex(NodeSelectorTreeViewWidget* wdg)
{
    auto rootItem = wdg->rootItem();
    return rootItem && rootItem->isCloudDrive();
}

QPushButton* SelectType::createCustomButton(const QString& type,
                                            const QString& text,
                                            const QString& iconFile)
{
    auto button(new TokenizableButton());
    button->setText(text);
    button->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
    QIcon icon;
    icon.addFile(iconFile, QSize(16, 16), QIcon::Mode::Normal, QIcon::State::Off);
    button->setIcon(icon);
    button->setProperty("type", type);
    button->setProperty("dimension", QLatin1String("small"));
    return button;
}

void DownloadType::init(NodeSelectorTreeViewWidget* wdg)
{
    wdg->ui->bNewFolder->hide();
    wdg->ui->tMegaFolders->setSelectionMode(QAbstractItemView::ExtendedSelection);
    wdg->mModel->showFiles(true);
    wdg->mModel->showReadOnlyFolders(true);
}

bool DownloadType::okButtonEnabled(NodeSelectorTreeViewWidget*, const QModelIndexList& selected)
{
    return !selected.isEmpty();
}

NodeSelectorModelItemSearch::Types DownloadType::allowedTypes()
{
    return NodeSelectorModelItemSearch::Type::CLOUD_DRIVE |
           NodeSelectorModelItemSearch::Type::INCOMING_SHARE |
           NodeSelectorModelItemSearch::Type::BACKUP;
}

void SyncType::init(NodeSelectorTreeViewWidget* wdg)
{
    wdg->mModel->setSyncSetupMode(true);
    wdg->mModel->showFiles(false);
    wdg->mModel->showReadOnlyFolders(false);
}

bool SyncType::okButtonEnabled(NodeSelectorTreeViewWidget*, const QModelIndexList& selected)
{
    if (selected.size() == 1)
    {
        bool isSyncable =
            selected.first().data(toInt(NodeSelectorModelRoles::IS_SYNCABLE_FOLDER_ROLE)).toBool();
        bool isFile = selected.first().data(toInt(NodeSelectorModelRoles::IS_FILE_ROLE)).toBool();
        return isSyncable && !isFile;
    }

    return false;
}

NodeSelectorModelItemSearch::Types SyncType::allowedTypes()
{
    return NodeSelectorModelItemSearch::Type::CLOUD_DRIVE |
           NodeSelectorModelItemSearch::Type::INCOMING_SHARE;
}

bool SyncType::isAllowedToNavigateInside(const QModelIndex& index)
{
    if (!SelectType::isAllowedToNavigateInside(index))
    {
        return false;
    }
    auto item = NodeSelectorModel::getItemByIndex(index);
    return !(item->getStatus() == NodeSelectorModelItem::Status::SYNC ||
             item->getStatus() == NodeSelectorModelItem::Status::SYNC_CHILD);
}

void StreamType::init(NodeSelectorTreeViewWidget* wdg)
{
    wdg->ui->bNewFolder->hide();
    wdg->mModel->showFiles(true);
    wdg->mModel->showReadOnlyFolders(true);
}

bool StreamType::okButtonEnabled(NodeSelectorTreeViewWidget*, const QModelIndexList& selected)
{
    if (selected.size() == 1)
    {
        return selected.first().data(toInt(NodeSelectorModelRoles::IS_FILE_ROLE)).toBool();
    }

    return false;
}

NodeSelectorModelItemSearch::Types StreamType::allowedTypes()
{
    return NodeSelectorModelItemSearch::Type::CLOUD_DRIVE |
           NodeSelectorModelItemSearch::Type::INCOMING_SHARE |
           NodeSelectorModelItemSearch::Type::BACKUP;
}

void UploadType::init(NodeSelectorTreeViewWidget* wdg)
{
    wdg->mModel->showFiles(false);
    wdg->mModel->showReadOnlyFolders(false);
}

bool UploadType::okButtonEnabled(NodeSelectorTreeViewWidget* wdg, const QModelIndexList& selected)
{
    if (selected.size() == 1)
    {
        return !selected.first().data(toInt(NodeSelectorModelRoles::IS_FILE_ROLE)).toBool();
    }

    return false;
}

NodeSelectorModelItemSearch::Types UploadType::allowedTypes()
{
    return NodeSelectorModelItemSearch::Type::CLOUD_DRIVE |
           NodeSelectorModelItemSearch::Type::INCOMING_SHARE;
}

//////////////////////////////////////////////////////////////////
void CloudDriveType::init(NodeSelectorTreeViewWidget* wdg)
{
    wdg->ui->bNewFolder->hide();
    wdg->ui->tMegaFolders->setSelectionMode(QAbstractItemView::ExtendedSelection);
    wdg->mModel->showFiles(true);
    wdg->mModel->showReadOnlyFolders(true);
}

NodeSelectorModelItemSearch::Types CloudDriveType::allowedTypes()
{
    return NodeSelectorModelItemSearch::Type::CLOUD_DRIVE |
           NodeSelectorModelItemSearch::Type::INCOMING_SHARE |
           NodeSelectorModelItemSearch::Type::BACKUP | NodeSelectorModelItemSearch::Type::RUBBISH;
}

bool CloudDriveType::footerVisible() const
{
    return false;
}

void CloudDriveType::makeViewCustomConnections(NodeSelectorTreeView* view,
                                               NodeSelectorTreeViewWidget* wdg)
{
    wdg->connect(view,
                 &NodeSelectorTreeView::uploadClicked,
                 wdg,
                 [wdg]()
                 {
                     emit wdg->onCustomButtonClicked(ButtonId::Upload);
                 });
}

bool CloudDriveType::okButtonEnabled(NodeSelectorTreeViewWidget*, const QModelIndexList& selected)
{
    return false;
}

void CloudDriveType::okCancelButtonsVisibility(NodeSelectorTreeViewWidget* wdg)
{
    wdg->ui->bOk->setVisible(false);
    wdg->ui->bCancel->setVisible(false);
}

QMap<uint, QPushButton*> CloudDriveType::addCustomButtons(NodeSelectorTreeViewWidget* wdg)
{
    auto& buttons = mCustomButtons[wdg];
    if (buttons.isEmpty())
    {
        auto uploadButton =
            createCustomButton(QLatin1String("ghost"),
                               getCustomButtonText(ButtonId::Upload),
                               Utilities::getPixmapName(QLatin1String("arrow-up"),
                                                        Utilities::AttributeType::SMALL |
                                                            Utilities::AttributeType::THIN |
                                                            Utilities::AttributeType::OUTLINE));

        buttons.insert(ButtonId::Upload, uploadButton);

        auto clearRubbishButton =
            createCustomButton(QLatin1String("ghost"),
                               getCustomButtonText(ButtonId::ClearRubbish),
                               Utilities::getPixmapName(QLatin1String("x"),
                                                        Utilities::AttributeType::SMALL |
                                                            Utilities::AttributeType::THIN |
                                                            Utilities::AttributeType::OUTLINE));
        buttons.insert(ButtonId::ClearRubbish, clearRubbishButton);
        clearRubbishButton->hide();
    }

    return buttons;
}

void CloudDriveType::updateCustomButtonsText(NodeSelectorTreeViewWidget* wdg)
{
    auto& buttons = mCustomButtons[wdg];
    if (!buttons.isEmpty())
    {
        for (auto it = buttons.keyValueBegin(); it != buttons.keyValueEnd(); ++it)
        {
            it->second->setText(getCustomButtonText(it->first));
        }
    }
}

void CloudDriveType::newFolderButtonVisibility(NodeSelectorTreeViewWidget* wdg)
{
    SelectType::newFolderButtonVisibility(wdg);
}

QString CloudDriveType::getCustomButtonText(uint buttonId) const
{
    switch (buttonId)
    {
        case ButtonId::Upload:
        {
            return MegaApplication::tr("Upload");
        }
        case ButtonId::ClearRubbish:
        {
            return NodeSelectorTreeViewWidget::tr("Empty Rubbish bin");
        }
        default:
        {
            return QString();
        }
    }
}

void CloudDriveType::selectionHasChanged(NodeSelectorTreeViewWidget* wdg)
{
    auto buttons = mCustomButtons.value(wdg);

    auto rubbishWidget = dynamic_cast<NodeSelectorTreeViewWidgetRubbish*>(wdg);

    if (rubbishWidget)
    {
        buttons.value(ButtonId::Upload)->setVisible(false);
        buttons.value(ButtonId::ClearRubbish)->setVisible(!rubbishWidget->isEmpty());
    }
    else
    {
        buttons.value(ButtonId::ClearRubbish)->setVisible(false);

        bool uploadEnabled(false);

        auto selected = wdg->getSelectedIndexes();

        if (!selected.isEmpty())
        {
            if (selected.size() == 1)
            {
                uploadEnabled =
                    !selected.first().data(toInt(NodeSelectorModelRoles::IS_FILE_ROLE)).toBool();
            }
        }

        buttons.value(ButtonId::Upload)
            ->setVisible(uploadEnabled && !wdg->isSelectionReadOnly(selected));
    }
}

//////////////////
NodeSelectorModelItemSearch::Types MoveBackupType::allowedTypes()
{
    return NodeSelectorModelItemSearch::Type::CLOUD_DRIVE;
}
