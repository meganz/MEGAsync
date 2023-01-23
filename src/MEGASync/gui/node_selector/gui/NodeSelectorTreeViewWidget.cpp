#include "NodeSelectorTreeViewWidget.h"
#include "ui_NodeSelectorTreeViewWidget.h"
#include "../model/NodeSelectorModel.h"
#include "../model/NodeSelectorDelegates.h"
#include "MegaApplication.h"
#include "QMegaMessageBox.h"
#include "../model/NodeSelectorProxyModel.h"
#include "../model/NodeSelectorModel.h"
#include "NodeNameSetterDialog/RenameNodeDialog.h"
#include "DialogOpener.h"

const int NodeSelectorTreeViewWidget::LABEL_ELIDE_MARGIN = 100;
const int NodeSelectorTreeViewWidget::LOADING_VIEW_THRESSHOLD = 500;


NodeSelectorTreeViewWidget::NodeSelectorTreeViewWidget(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::NodeSelectorTreeViewWidget),
    mProxyModel(nullptr),
    mSelectMode(UNINITIALIZED_SELECT),
    mMegaApi(MegaSyncApp->getMegaApi()),
    mManuallyResizedColumn(false),
    mDelegateListener(new QTMegaRequestListener(mMegaApi, this)),
    mModel(nullptr),
    first(true),
    mUiBlocked(false),
    mNodeHandleToSelect(INVALID_HANDLE)
{
    ui->setupUi(this);

    ui->cbAlwaysUploadToLocation->hide();
    ui->bOk->setDefault(true);
    ui->bOk->setEnabled(false);

    connect(ui->bNewFolder, &QPushButton::clicked, this, &NodeSelectorTreeViewWidget::onbNewFolderClicked);
    connect(ui->bOk, &QPushButton::clicked, this, &NodeSelectorTreeViewWidget::okBtnClicked);
    connect(ui->bCancel, &QPushButton::clicked, this, &NodeSelectorTreeViewWidget::cancelBtnClicked);
    connect(ui->cbAlwaysUploadToLocation, &QCheckBox::stateChanged, this, &NodeSelectorTreeViewWidget::oncbAlwaysUploadToLocationChanged);
    checkBackForwardButtons();

    mLoadingScene.setView(ui->tMegaFolders);
    mLoadingScene.setDelayTimeToShowInMs(100);
    connect(&mLoadingScene, &ViewLoadingSceneBase::sceneVisibilityChange, this, &NodeSelectorTreeViewWidget::onUiBlocked);
}

void NodeSelectorTreeViewWidget::changeEvent(QEvent *event)
{
    if (event->type() == QEvent::LanguageChange)
    {
        if(!ui->tMegaFolders->rootIndex().isValid())
        {
            ui->lFolderName->setText(getRootText());
        }
    }
    QWidget::changeEvent(event);
}

void NodeSelectorTreeViewWidget::setSelectionMode(Type selectMode)
{
    if(mSelectMode != UNINITIALIZED_SELECT)
    {
        return;
    }

    mSelectMode = selectMode;

    mProxyModel = std::unique_ptr<NodeSelectorProxyModel>(new NodeSelectorProxyModel());
    mModel = getModel();

    switch(mSelectMode)
    {
        case SYNC_SELECT:
            mModel->setSyncSetupMode(true);
            // fall through
        case UPLOAD_SELECT:
            ui->bNewFolder->show();
            mProxyModel->showReadOnlyFolders(false);
            mModel->showFiles(false);
            break;
        case DOWNLOAD_SELECT:
            ui->bNewFolder->hide();
            ui->tMegaFolders->setSelectionMode(QAbstractItemView::ExtendedSelection);
            mProxyModel->showReadOnlyFolders(true);
            mModel->showFiles(true);
            break;
        case STREAM_SELECT:
            ui->bNewFolder->hide();
            mProxyModel->showReadOnlyFolders(true);
            mModel->showFiles(true);
            break;
        default:
            break;
    }

    connect(mProxyModel.get(), &NodeSelectorProxyModel::expandReady, this, &NodeSelectorTreeViewWidget::onExpandReady);

    ui->tMegaFolders->setSortingEnabled(true);
    mProxyModel->setSourceModel(mModel.get());

#ifdef __APPLE__
ui->tMegaFolders->setAnimated(false);
#endif
}

void NodeSelectorTreeViewWidget::showDefaultUploadOption(bool show)
{
    ui->cbAlwaysUploadToLocation->setVisible(show);
}

void NodeSelectorTreeViewWidget::abort()
{
    mModel->abort();
}

NodeSelectorProxyModel* NodeSelectorTreeViewWidget::getProxyModel()
{
    return mProxyModel.get();
}

void NodeSelectorTreeViewWidget::setDefaultUploadOption(bool value)
{
    ui->cbAlwaysUploadToLocation->setChecked(value);
}

bool NodeSelectorTreeViewWidget::getDefaultUploadOption()
{
    return ui->cbAlwaysUploadToLocation->isChecked();
}
void NodeSelectorTreeViewWidget::setTitle(const QString &title)
{
    ui->lFolderName->setText(title);
}

void NodeSelectorTreeViewWidget::mousePressEvent(QMouseEvent *event)
{
    if(event->button() == Qt::BackButton && ui->bBack->isEnabled())
    {
       onGoBackClicked();
    }
    else if(event->button() == Qt::ForwardButton && ui->bForward->isEnabled())
    {
       onGoForwardClicked();
    }
}

void NodeSelectorTreeViewWidget::showEvent(QShowEvent* )
{
    ui->tMegaFolders->setColumnWidth(NodeSelectorModel::COLUMN::NODE, qRound(ui->tMegaFolders->width() * 0.57));
}

void NodeSelectorTreeViewWidget::resizeEvent(QResizeEvent *)
{
    if(!mManuallyResizedColumn)
    {
        ui->tMegaFolders->setColumnWidth(NodeSelectorModel::COLUMN::NODE, qRound(ui->tMegaFolders->width() * 0.57));
    }
}

void NodeSelectorTreeViewWidget::onSectionResized()
{
    if(!mManuallyResizedColumn
            && ui->tMegaFolders->header()->rect().contains(ui->tMegaFolders->mapFromGlobal(QCursor::pos())))
    {
        mManuallyResizedColumn = true;
    }
}

void NodeSelectorTreeViewWidget::onRowsInserted()
{
    if(first)
    {
        first = false;
        setSelectedNodeHandle(mNodeHandleToSelect);
    }
}

void NodeSelectorTreeViewWidget::onExpandReady()
{
    if(ui->tMegaFolders->model() == nullptr)
    {
        ui->tMegaFolders->setContextMenuPolicy(Qt::DefaultContextMenu);
        ui->tMegaFolders->setExpandsOnDoubleClick(false);
        ui->tMegaFolders->setHeader(new NodSelectorTreeViewHeaderView(Qt::Horizontal));
        ui->tMegaFolders->setItemDelegate(new NodeRowDelegate(ui->tMegaFolders));
        ui->tMegaFolders->setItemDelegateForColumn(NodeSelectorModel::STATUS, new IconDelegate(ui->tMegaFolders));
        ui->tMegaFolders->setItemDelegateForColumn(NodeSelectorModel::USER, new IconDelegate(ui->tMegaFolders));
        ui->tMegaFolders->setTextElideMode(Qt::ElideMiddle);

        ui->tMegaFolders->sortByColumn(NodeSelectorModel::NODE, Qt::AscendingOrder);
        ui->tMegaFolders->setModel(mProxyModel.get());

        ui->tMegaFolders->header()->show();
        ui->tMegaFolders->header()->setFixedHeight(NodeSelectorModel::ROW_HEIGHT);
        ui->tMegaFolders->header()->moveSection(NodeSelectorModel::STATUS, NodeSelectorModel::NODE);
        ui->tMegaFolders->setColumnWidth(NodeSelectorModel::COLUMN::STATUS, NodeSelectorModel::ROW_HEIGHT * 2);
        ui->tMegaFolders->header()->setProperty("HeaderIconCenter", true);
        showEvent(nullptr);

        //those connects needs to be done after the model is set, do not move them

        connect(ui->tMegaFolders->selectionModel(), &QItemSelectionModel::selectionChanged, this, &NodeSelectorTreeViewWidget::onSelectionChanged);
        connect(ui->tMegaFolders, &NodeSelectorTreeView::removeNodeClicked, this, &NodeSelectorTreeViewWidget::onDeleteClicked);
        connect(ui->tMegaFolders, &NodeSelectorTreeView::renameNodeClicked, this, &NodeSelectorTreeViewWidget::onRenameClicked);
        connect(ui->tMegaFolders, &NodeSelectorTreeView::getMegaLinkClicked, this, &NodeSelectorTreeViewWidget::onGenMEGALinkClicked);
        connect(ui->tMegaFolders, &QTreeView::doubleClicked, this, &NodeSelectorTreeViewWidget::onItemDoubleClick);
        connect(ui->bForward, &QPushButton::clicked, this, &NodeSelectorTreeViewWidget::onGoForwardClicked);
        connect(ui->bBack, &QPushButton::clicked, this, &NodeSelectorTreeViewWidget::onGoBackClicked);
        connect(ui->tMegaFolders->header(), &QHeaderView::sectionResized, this, &NodeSelectorTreeViewWidget::onSectionResized);
        connect(mModel.get(), &QAbstractItemModel::rowsInserted, this, &NodeSelectorTreeViewWidget::onRowsInserted);

        connect(mModel.get(), &NodeSelectorModel::blockUi, this, &NodeSelectorTreeViewWidget::setLoadingSceneVisible);

        setRootIndex(QModelIndex());
        checkNewFolderButtonVisibility();

        emit onViewReady(mProxyModel->rowCount() == 0);
    }

    auto indexesAndSelected = mModel->needsToBeExpandedAndSelected();
    if(!indexesAndSelected.indexesToBeExpanded.isEmpty())
    {
        for (auto it = indexesAndSelected.indexesToBeExpanded.begin(); it != indexesAndSelected.indexesToBeExpanded.end(); ++it)
        {
             auto proxyIndex(mProxyModel->mapFromSource((*it)));
             ui->tMegaFolders->setExpanded(proxyIndex, true);

             if((*it) == indexesAndSelected.indexesToBeExpanded.last())
             {
                 if(indexesAndSelected.needsToBeSelected)
                 {
                     ui->tMegaFolders->selectionModel()->setCurrentIndex(proxyIndex, QItemSelectionModel::ClearAndSelect | QItemSelectionModel::Rows);
                     ui->tMegaFolders->selectionModel()->select(proxyIndex, QItemSelectionModel::ClearAndSelect | QItemSelectionModel::Rows);
                     ui->tMegaFolders->scrollTo(proxyIndex, QAbstractItemView::ScrollHint::PositionAtCenter);
                 }

                 if(indexesAndSelected.needsToBeEntered)
                 {
                     onItemDoubleClick(proxyIndex);
                 }
             }
        }
    }
}

void NodeSelectorTreeViewWidget::onGoBackClicked()
{
    mNavigationInfo.appendToForward(getHandleByIndex(ui->tMegaFolders->rootIndex()));
    QModelIndex indexToGo = getIndexFromHandle(mNavigationInfo.backwardHandles.last());
    mNavigationInfo.backwardHandles.removeLast();

    setRootIndex(indexToGo);
    checkBackForwardButtons();
    checkNewFolderButtonVisibility();
}

void NodeSelectorTreeViewWidget::onGoForwardClicked()
{
    mNavigationInfo.appendToBackward(getHandleByIndex(ui->tMegaFolders->rootIndex()));
    QModelIndex indexToGo = getIndexFromHandle(mNavigationInfo.forwardHandles.last());
    mNavigationInfo.forwardHandles.removeLast();
    setRootIndex(indexToGo);
    checkBackForwardButtons();
    checkNewFolderButtonVisibility();
}

MegaHandle NodeSelectorTreeViewWidget::getHandleByIndex(const QModelIndex& idx)
{
    return mProxyModel ? mProxyModel->getHandle(idx) : mega::INVALID_HANDLE;
}

QModelIndex NodeSelectorTreeViewWidget::getIndexFromHandle(const mega::MegaHandle &handle)
{
    return mProxyModel ? mProxyModel->getIndexFromHandle(handle) : QModelIndex();
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

    QPointer<NewFolderDialog> dialog(new NewFolderDialog(parentNode, this));
    DialogOpener::showDialog(dialog,  [this, dialog]()
    {
        auto newNode = dialog->getNewNode();
        //IF the dialog return a node, there are two scenarios:
        //1) The dialog has been accepted, a new folder has been created
        //2) The dialog has been rejected because the folder already exists. If so, select the existing folder
        if(newNode)
        {
            if(dialog->result() == QDialog::Accepted)
            {
                QModelIndex idx = ui->tMegaFolders->rootIndex();
                if(!idx.isValid())
                {
                    idx = mProxyModel->getIndexFromNode(MegaSyncApp->getRootNode());
                }
                mProxyModel->setExpandMapped(true);
                mProxyModel->addNode(std::move(newNode), idx);
            }
        }
    });
}

void NodeSelectorTreeViewWidget::oncbAlwaysUploadToLocationChanged(bool value)
{
    foreach(auto& child, parent()->children())
    {
        if(auto tvw = qobject_cast<NodeSelectorTreeViewWidget*>(child))
        {
            if(tvw != sender())
            {
                tvw->setDefaultUploadOption(value);
            }
        }
    }
}

bool NodeSelectorTreeViewWidget::isAllowedToEnterInIndex(const QModelIndex &idx)
{
    auto source_idx = mProxyModel->getIndexFromSource(idx);
    NodeSelectorModelItem *item = static_cast<NodeSelectorModelItem*>(source_idx.internalPointer());
    if(item)
    {
        if((item->getNode()->isFile())
           || (item->isCloudDrive())
           || (mSelectMode == SYNC_SELECT
               && (item->getStatus() == NodeSelectorModelItem::SYNC
                   || item->getStatus() == NodeSelectorModelItem::SYNC_CHILD)))
        {
            return false;
        }
    }
    return true;
}

void NodeSelectorTreeViewWidget::onItemDoubleClick(const QModelIndex &index)
{
    if(!isAllowedToEnterInIndex(index) )
        return;

    mNavigationInfo.appendToBackward(getHandleByIndex(ui->tMegaFolders->rootIndex()));
    mNavigationInfo.removeFromForward(mProxyModel->getHandle(index));

    setRootIndex(index);
    checkBackForwardButtons();
    checkNewFolderButtonVisibility();
}

void NodeSelectorTreeViewWidget::checkNewFolderButtonVisibility()
{
    if(mSelectMode == SYNC_SELECT || mSelectMode == UPLOAD_SELECT)
    {
        auto sourceIndex = mProxyModel->getIndexFromSource(ui->tMegaFolders->rootIndex());
        ui->bNewFolder->setVisible(sourceIndex.isValid() || newFolderBtnVisibleInRoot());
    }
}

void NodeSelectorTreeViewWidget::setLoadingSceneVisible(bool blockUi)
{
    ui->tMegaFolders->blockSignals(blockUi);
    ui->tMegaFolders->header()->blockSignals(blockUi);

    mLoadingScene.toggleLoadingScene(blockUi);
}

void NodeSelectorTreeViewWidget::onUiBlocked(bool state)
{
    if(mUiBlocked != state)
    {
        mUiBlocked = state;

        ui->bNewFolder->setDisabled(state);
        ui->bCancel->setDisabled(state);

        if(!state)
        {
            auto selection = ui->tMegaFolders->selectionModel()->selectedIndexes();
            checkOkButton(selection);
            checkBackForwardButtons();
        }
        else
        {
            ui->bBack->setEnabled(false);
            ui->bForward->setEnabled(false);
            ui->bOk->setDisabled(true);
        }
    }
}

void NodeSelectorTreeViewWidget::onSelectionChanged(const QItemSelection& selected, const QItemSelection& deselected)
{
    Q_UNUSED(deselected)

    if(!mUiBlocked)
    {
        checkOkButton(selected.indexes());
    }
}

void NodeSelectorTreeViewWidget::checkOkButton(const QModelIndexList &selected)
{
    auto result(false);

    if(!selected.isEmpty())
    {
        if(mSelectMode == UPLOAD_SELECT || mSelectMode == DOWNLOAD_SELECT)
        {
            result = true;
        }
        else
        {
            int correctSelected(0);

            foreach(auto& index, selected)
            {
                auto source_idx = mProxyModel->getIndexFromSource(index);
                NodeSelectorModelItem *item = static_cast<NodeSelectorModelItem*>(source_idx.internalPointer());
                if(item)
                {
                    if(mSelectMode == STREAM_SELECT)
                    {
                        item->getNode()->isFile() ? correctSelected++ : correctSelected;
                    }
                    else if(mSelectMode == SYNC_SELECT)
                    {
                        item->getNode()->isFolder() ? correctSelected++ : correctSelected;
                    }
                }
            }

            result = correctSelected == selected.size();
        }
    }

    ui->bOk->setEnabled(result);
}

void NodeSelectorTreeViewWidget::onRenameClicked()
{
    auto node = std::unique_ptr<MegaNode>(mMegaApi->getNodeByHandle(getSelectedNodeHandle()));
    int access = mMegaApi->getAccess(node.get());
    if (!node || access < MegaShare::ACCESS_FULL)
    {
        return;
    }

    QPointer<RenameRemoteNodeDialog> dialog(new RenameRemoteNodeDialog(std::move(node), nullptr));
    DialogOpener::showDialog(dialog, [this, dialog]
    {
        if(dialog->result() == QDialog::Accepted)
        {
            auto selectedIndex = getSelectedIndex();
            if(selectedIndex.isValid())
            {
                auto sourceIndex = mProxyModel->mapToSource(selectedIndex);
                NodeSelectorModelItem *item = static_cast<NodeSelectorModelItem*>(sourceIndex.internalPointer());
                if(item)
                {
                    auto updatedNode = std::shared_ptr<MegaNode>(mMegaApi->getNodeByHandle(getSelectedNodeHandle()));
                    item->updateNode(updatedNode);
                }
            }
        }
     });
}

void NodeSelectorTreeViewWidget::onDeleteClicked()
{
    auto node = std::unique_ptr<MegaNode>(mMegaApi->getNodeByHandle(getSelectedNodeHandle()));
    int access = mMegaApi->getAccess(node.get());
    if (!node || access < MegaShare::ACCESS_FULL)
    {
        return;
    }

    QPointer<NodeSelectorTreeViewWidget> currentDialog = this;
    if (QMegaMessageBox::question(this,
                             QLatin1String("MEGAsync"),
                             tr("Are you sure that you want to delete \"%1\"?")
                                .arg(QString::fromUtf8(node->getName())),
                             QMessageBox::Yes | QMessageBox::No, QMessageBox::No) == QMessageBox::Yes)
    {
        if (!currentDialog)
        {
            return;
        }

        ui->tMegaFolders->setEnabled(false);
        ui->bBack->setEnabled(false);
        ui->bForward->setEnabled(false);
        ui->bNewFolder->setEnabled(false);
        ui->bOk->setEnabled(false);
        const char *name = node->getName();
        if (access == MegaShare::ACCESS_FULL
                || !strcmp(name, "NO_KEY")
                || !strcmp(name, "CRYPTO_ERROR")
                || !strcmp(name, "BLANK"))
        {
            mMegaApi->remove(node.get(), mDelegateListener.get());
        }
        else
        {
            auto rubbish = MegaSyncApp->getRubbishNode();
            mMegaApi->moveNode(node.get(), rubbish.get(), mDelegateListener.get());
        }
    }
}

void NodeSelectorTreeViewWidget::onRequestFinish(MegaApi *, MegaRequest *request, MegaError *e)
{
    ui->bNewFolder->setEnabled(true);
    ui->bOk->setEnabled(true);

    if (e->getErrorCode() != MegaError::API_OK)
    {
        ui->tMegaFolders->setEnabled(true);
        QMegaMessageBox::critical(nullptr, QLatin1String("MEGAsync"), tr("Error:") + QLatin1String(" ") + QCoreApplication::translate("MegaError", e->getErrorString()));
        return;
    }

    if (request->getType() == MegaRequest::TYPE_REMOVE || request->getType() == MegaRequest::TYPE_MOVE)
    {
        if (e->getErrorCode() == MegaError::API_OK)
        {
            auto selectedIndex = getSelectedIndex();
            if(!selectedIndex.isValid())
              return;

            auto parent = mProxyModel->getNode(selectedIndex.parent());
            mNavigationInfo.remove(mProxyModel->getHandle(selectedIndex));
            mProxyModel->removeNode(selectedIndex);
            if(parent)
                setSelectedNodeHandle(parent->getHandle());
        }
    }
    checkBackForwardButtons();
    ui->tMegaFolders->setEnabled(true);
}

void NodeSelectorTreeViewWidget::setSelectedNodeHandle(const MegaHandle& selectedHandle)
{
    if(selectedHandle == INVALID_HANDLE)
    {
        return;
    }

    auto node = std::shared_ptr<MegaNode>(mMegaApi->getNodeByHandle(selectedHandle));
    if (!node)
        return;

    mModel->loadTreeFromNode(node);
}

void NodeSelectorTreeViewWidget::setFutureSelectedNodeHandle(const mega::MegaHandle &selectedHandle)
{
    mNodeHandleToSelect = selectedHandle;
}

MegaHandle NodeSelectorTreeViewWidget::getSelectedNodeHandle()
{
    return ui->tMegaFolders->getSelectedNodeHandle();
}

QList<MegaHandle> NodeSelectorTreeViewWidget::getMultiSelectionNodeHandle()
{
    QList<MegaHandle> ret;
    foreach(auto& s_index, ui->tMegaFolders->selectionModel()->selectedRows())
    {
        if(auto node = mProxyModel->getNode(s_index))
            ret.append(node->getHandle());
    }
    return ret;
}

QModelIndex NodeSelectorTreeViewWidget::getSelectedIndex()
{
    QModelIndex ret;
    if(ui->tMegaFolders->selectionModel()->selectedRows().size() > 0)
        ret = ui->tMegaFolders->selectionModel()->selectedRows().at(0);
    return ret;
}

void NodeSelectorTreeViewWidget::checkBackForwardButtons()
{
    ui->bBack->setEnabled(!mNavigationInfo.backwardHandles.isEmpty());
    ui->bForward->setEnabled(!mNavigationInfo.forwardHandles.isEmpty());
}

void NodeSelectorTreeViewWidget::setRootIndex(const QModelIndex &proxy_idx)
{
    //In case the idx is coming from a potentially hidden column, we always take the NODE column
    //As it is the only one that have childrens
    auto node_column_idx = proxy_idx.sibling(proxy_idx.row(), NodeSelectorModel::COLUMN::NODE);

    ui->tMegaFolders->setRootIndex(node_column_idx);

    auto source_idx = mProxyModel->getIndexFromSource(node_column_idx);
    onRootIndexChanged(source_idx);

    if(!node_column_idx.isValid())
    {
        ui->lFolderName->setText(getRootText());

        QModelIndexList selectedIndexes = ui->tMegaFolders->selectionModel()->selectedIndexes();
        foreach(auto& selection, selectedIndexes)
        {
            ui->tMegaFolders->selectionModel()->select(selection, QItemSelectionModel::ClearAndSelect | QItemSelectionModel::Rows);
        }
        ui->lFolderName->setToolTip(QString());
        ui->lOwnerIcon->setPixmap(QPixmap());
        ui->avatarSpacer->spacerItem()->changeSize(0, 0);
        ui->lIcon->setPixmap(QPixmap());
        ui->syncSpacer->spacerItem()->changeSize(0, 0);
        return;
    }

    if(!source_idx.isValid())
    {
        ui->lOwnerIcon->setPixmap(QPixmap());
        ui->lIcon->setPixmap(QPixmap());
        return;
    }

    //Taking the sync icon
    auto status_column_idx = proxy_idx.sibling(proxy_idx.row(), NodeSelectorModel::COLUMN::STATUS);
    QIcon syncIcon = qvariant_cast<QIcon>(status_column_idx.data(Qt::DecorationRole));

    NodeSelectorModelItem *item = static_cast<NodeSelectorModelItem*>(source_idx.internalPointer());
    if(!item)
        return;

    if(!syncIcon.isNull())
    {
        QPixmap pm = syncIcon.pixmap(QSize(NodeSelectorModelItem::ICON_SIZE, NodeSelectorModelItem::ICON_SIZE), QIcon::Normal);
        ui->lIcon->setPixmap(pm);
        ui->syncSpacer->spacerItem()->changeSize(10, 0);
    }
    else
    {
        ui->lIcon->setPixmap(QPixmap());
        ui->syncSpacer->spacerItem()->changeSize(0, 0);
    }

    auto node = item->getNode();
    if(node)
    {
        QString nodeName = QString::fromUtf8(node->getName());
        QFontMetrics fm = ui->lFolderName->fontMetrics();

        if(nodeName == QLatin1String("NO_KEY") || nodeName == QLatin1String("CRYPTO_ERROR"))
        {
            nodeName = QCoreApplication::translate("MegaError", "Decryption error");
        }

        QString elidedText = fm.elidedText(nodeName, Qt::ElideMiddle, ui->tMegaFolders->width() - LABEL_ELIDE_MARGIN);
        ui->lFolderName->setText(elidedText);

        if(elidedText != nodeName)
            ui->lFolderName->setToolTip(nodeName);
        else
            ui->lFolderName->setToolTip(QString());
    }
}

QModelIndex NodeSelectorTreeViewWidget::getParentIncomingShareByIndex(QModelIndex idx)
{
    while(idx.isValid())
    {
        if(NodeSelectorModelItem *item = static_cast<NodeSelectorModelItem*>(idx.internalPointer()))
        {
            if(item->getNode()->isInShare())
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


void NodeSelectorTreeViewWidget::onGenMEGALinkClicked()
{
    auto node = std::unique_ptr<MegaNode>(mMegaApi->getNodeByHandle(getSelectedNodeHandle()));
    if (!node || node->getType() == MegaNode::TYPE_ROOT
            || mMegaApi->getAccess(node.get()) != MegaShare::ACCESS_OWNER)
    {
        return;
    }
    mMegaApi->exportNode(node.get());
}

NodeSelectorTreeViewWidget::~NodeSelectorTreeViewWidget()
{
    setLoadingSceneVisible(false);
    delete ui;
}

void NodeSelectorTreeViewWidget::Navigation::removeFromForward(const mega::MegaHandle &handle)
{
    if(forwardHandles.isEmpty())
        return;

    auto megaApi = MegaSyncApp->getMegaApi();
    auto p_node = std::unique_ptr<mega::MegaNode>(megaApi->getNodeByHandle(handle));

    QMap<MegaHandle, MegaHandle> parentHandles;
    while(p_node)
    {
        MegaHandle actualHandle = p_node->getHandle();
        p_node.reset(megaApi->getParentNode(p_node.get()));
        MegaHandle parentHandle= INVALID_HANDLE;
        if(p_node)
            parentHandle = p_node->getHandle();
        parentHandles.insert(parentHandle, actualHandle);
    }

    p_node.reset(megaApi->getNodeByHandle(forwardHandles.last()));
    QMap<MegaHandle, MegaHandle> actualListParentHandles;
    while(p_node)
    {
        MegaHandle actualHandle = p_node->getHandle();
        p_node.reset(megaApi->getParentNode(p_node.get()));
        MegaHandle parentHandle= INVALID_HANDLE;
        if(p_node)
            parentHandle = p_node->getHandle();
        actualListParentHandles.insert(parentHandle, actualHandle);
    }

    for(auto it = actualListParentHandles.begin(); it != actualListParentHandles.end(); ++it)
    {
        if(parentHandles.contains(it.key()))
        {
            forwardHandles.clear();
            return;
        }
    }
}

void NodeSelectorTreeViewWidget::Navigation::remove(const mega::MegaHandle &handle)
{
    backwardHandles.removeAll(handle);
    forwardHandles.removeAll(handle);
}

void NodeSelectorTreeViewWidget::Navigation::appendToBackward(const mega::MegaHandle &handle)
{
    if(!backwardHandles.contains(handle))
        backwardHandles.append(handle);
}

void NodeSelectorTreeViewWidget::Navigation::appendToForward(const mega::MegaHandle &handle)
{
    if(!forwardHandles.contains(handle))
        forwardHandles.append(handle);
}
