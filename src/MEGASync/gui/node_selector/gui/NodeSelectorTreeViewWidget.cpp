#include "ui_NodeSelectorTreeViewWidget.h"
#include "NodeSelectorTreeViewWidget.h"
#include "NodeSelectorTreeViewWidgetSpecializations.h"
#include "../model/NodeSelectorModel.h"
#include "../model/NodeSelectorDelegates.h"
#include "MegaApplication.h"
#include "QMegaMessageBox.h"
#include "../model/NodeSelectorProxyModel.h"
#include "../model/NodeSelectorModel.h"
#include "NodeNameSetterDialog/RenameNodeDialog.h"
#include "DialogOpener.h"
#include <MegaNodeNames.h>
#include "NodeNameSetterDialog/NewFolderDialog.h"

const int NodeSelectorTreeViewWidget::LOADING_VIEW_THRESSHOLD = 500;
const int NodeSelectorTreeViewWidget::LABEL_ELIDE_MARGIN = 250;
const char* NodeSelectorTreeViewWidget::FULL_NAME_PROPERTY = "full_name";

NodeSelectorTreeViewWidget::NodeSelectorTreeViewWidget(SelectTypeSPtr mode, QWidget *parent) :
    QWidget(parent),
    ui(new Ui::NodeSelectorTreeViewWidget),
    mProxyModel(nullptr),
    mModel(nullptr),
    mMegaApi(MegaSyncApp->getMegaApi()),
    mManuallyResizedColumn(false),
    mDelegateListener(new QTMegaRequestListener(mMegaApi, this)),
    first(true),
    mUiBlocked(false),
    mNodeHandleToSelect(INVALID_HANDLE),
    mSelectType(mode)
{
    ui->setupUi(this);
    setFocusProxy(ui->tMegaFolders);
    ui->cbAlwaysUploadToLocation->hide();
    ui->bOk->setDefault(true);
    ui->bOk->setEnabled(false);
    ui->searchButtonsWidget->setVisible(false);
    ui->searchingText->setVisible(false);

    connect(ui->bNewFolder, &QPushButton::clicked, this, &NodeSelectorTreeViewWidget::onbNewFolderClicked);
    connect(ui->bOk, &QPushButton::clicked, this, &NodeSelectorTreeViewWidget::okBtnClicked);
    connect(ui->bCancel, &QPushButton::clicked, this, &NodeSelectorTreeViewWidget::cancelBtnClicked);
    connect(ui->cbAlwaysUploadToLocation, &QCheckBox::stateChanged, this, &NodeSelectorTreeViewWidget::oncbAlwaysUploadToLocationChanged);
    connect(ui->leSearch, &SearchLineEdit::search, this, &NodeSelectorTreeViewWidget::onSearch);
    checkBackForwardButtons();

    mLoadingScene.setView(ui->tMegaFolders);
    connect(&mLoadingScene, &ViewLoadingSceneBase::sceneVisibilityChange, this, &NodeSelectorTreeViewWidget::onUiBlocked);

    foreach(auto& button, ui->searchButtonsWidget->findChildren<QAbstractButton*>())
    {
        button->setProperty(ButtonIconManager::CHANGE_LATER, true);
        mButtonIconManager.addButton(button);
    }
}

NodeSelectorTreeViewWidget::~NodeSelectorTreeViewWidget()
{
    delete ui;
}

void NodeSelectorTreeViewWidget::changeEvent(QEvent *event)
{
    if (event->type() == QEvent::LanguageChange)
    {
        if(!ui->tMegaFolders->rootIndex().isValid())
        {
            ui->lFolderName->setText(getRootText());
        }
        ui->retranslateUi(this);
    }
    QWidget::changeEvent(event);
}

void NodeSelectorTreeViewWidget::init()
{
    mProxyModel = createProxyModel();
    mModel = createModel();
    ui->emptyIcon->setIcon(getEmptyIcon());
    mSelectType->init(this);

    connect(mProxyModel.get(), &NodeSelectorProxyModel::expandReady, this, &NodeSelectorTreeViewWidget::onExpandReady);
    connect(mModel.get(), &QAbstractItemModel::rowsInserted, this, &NodeSelectorTreeViewWidget::onRowsInserted);
    connect(mModel.get(), &NodeSelectorModel::blockUi, this, &NodeSelectorTreeViewWidget::setLoadingSceneVisible);

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

void NodeSelectorTreeViewWidget::setSearchText(const QString &text)
{
    ui->leSearch->setText(text);
}

void NodeSelectorTreeViewWidget::setTitleText(const QString &nodeName)
{
    ui->lFolderName->setProperty(FULL_NAME_PROPERTY, nodeName);

    QFontMetrics fm = ui->lFolderName->fontMetrics();

    QString elidedText = fm.elidedText(nodeName, Qt::ElideMiddle, ui->tMegaFolders->width() - LABEL_ELIDE_MARGIN);
    ui->lFolderName->setText(elidedText);

    if(elidedText != nodeName)
        ui->lFolderName->setToolTip(nodeName);
    else
        ui->lFolderName->setToolTip(QString());

}

void NodeSelectorTreeViewWidget::clearSearchText()
{
    ui->leSearch->onClearClicked();
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
    if(!mManuallyResizedColumn)
    {
        ui->tMegaFolders->setColumnWidth(NodeSelectorModel::COLUMN::NODE, qRound(ui->stackedWidget->width() * 0.50));
    }
}

void NodeSelectorTreeViewWidget::resizeEvent(QResizeEvent *)
{
    if(!mManuallyResizedColumn)
    {
        ui->tMegaFolders->setColumnWidth(NodeSelectorModel::COLUMN::NODE, qRound(ui->stackedWidget->width() * 0.50));
    }

    setTitleText(ui->lFolderName->property(FULL_NAME_PROPERTY).toString());
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
        ui->tMegaFolders->setHeader(new NodeSelectorTreeViewHeaderView(Qt::Horizontal));
        ui->tMegaFolders->setItemDelegate(new NodeRowDelegate(ui->tMegaFolders));
        ui->tMegaFolders->setItemDelegateForColumn(NodeSelectorModel::STATUS, new IconDelegate(ui->tMegaFolders));
        ui->tMegaFolders->setItemDelegateForColumn(NodeSelectorModel::USER, new IconDelegate(ui->tMegaFolders));
        ui->tMegaFolders->setItemDelegateForColumn(NodeSelectorModel::DATE, new DateColumnDelegate(ui->tMegaFolders));
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
        connect(ui->tMegaFolders, &NodeSelectorTreeView::nodeSelected, this, &NodeSelectorTreeViewWidget::okBtnClicked);
        connect(ui->bForward, &QPushButton::clicked, this, &NodeSelectorTreeViewWidget::onGoForwardClicked);
        connect(ui->bBack, &QPushButton::clicked, this, &NodeSelectorTreeViewWidget::onGoBackClicked);
        connect(ui->tMegaFolders->header(), &QHeaderView::sectionResized, this, &NodeSelectorTreeViewWidget::onSectionResized);

        setRootIndex(QModelIndex());
        checkNewFolderButtonVisibility();
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
    dialog->init();
    DialogOpener::showDialog(dialog,  [this, dialog]()
    {
        auto newNode = dialog->getNewNode();
        //IF the dialog return a node, there are two scenarios:
        //1) The dialog has been accepted, a new folder has been created
        //2) The dialog has been rejected because the folder already exists. If so, select the existing folder
        if(newNode)
        {
            //Set the focus to the view to allow the user to press enter (or go back, in a future feature)
            ui->tMegaFolders->setFocus();
            QModelIndex idx = ui->tMegaFolders->rootIndex();
            if(!idx.isValid())
            {
                QModelIndex idx = ui->tMegaFolders->rootIndex();
                if(!idx.isValid())
                {
                    idx = mProxyModel->getIndexFromNode(MegaSyncApp->getRootNode());
                }
                mProxyModel->setExpandMapped(true);
                mProxyModel->addNode(std::move(newNode), idx);
                ui->bOk->setFocus();
            }
            mProxyModel->setExpandMapped(true);
            mProxyModel->addNode(std::move(newNode), idx);
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
    return mSelectType->isAllowedToNavigateInside(idx);
}

void NodeSelectorTreeViewWidget::onItemDoubleClick(const QModelIndex &index)
{
    if(!isAllowedToEnterInIndex(index))
        return;

    mNavigationInfo.appendToBackward(getHandleByIndex(ui->tMegaFolders->rootIndex()));
    mNavigationInfo.removeFromForward(mProxyModel->getHandle(index));

    setRootIndex(index);
    checkBackForwardButtons();
    checkNewFolderButtonVisibility();
}

void NodeSelectorTreeViewWidget::checkNewFolderButtonVisibility()
{
    mSelectType->newFolderButtonVisibility(this);
}

std::unique_ptr<NodeSelectorProxyModel> NodeSelectorTreeViewWidget::createProxyModel()
{
    return std::unique_ptr<NodeSelectorProxyModel>(new NodeSelectorProxyModel);
}

void NodeSelectorTreeViewWidget::setLoadingSceneVisible(bool blockUi)
{
    ui->tMegaFolders->blockSignals(blockUi);
    ui->tMegaFolders->header()->blockSignals(blockUi);
    mLoadingScene.toggleLoadingScene(blockUi);

    if(!blockUi)
    {
        modelLoaded();
    }
}

void NodeSelectorTreeViewWidget::modelLoaded()
{
    if(mModel)
    {
        if(mModel->rowCount() == 0)
        {
            ui->stackedWidget->setCurrentWidget(ui->emptyPage);
            return;
        }
    }
    ui->stackedWidget->setCurrentWidget(ui->treeViewPage);
}


void NodeSelectorTreeViewWidget::onUiBlocked(bool state)
{
    if(mUiBlocked != state)
    {
        mUiBlocked = state;

        ui->bNewFolder->setDisabled(state);
        ui->bCancel->setDisabled(state);
        ui->searchButtonsWidget->setDisabled(state);

        if(!state)
        {
            if(auto selectionModel = ui->tMegaFolders->selectionModel())
            {
                checkOkButton(selectionModel->selectedRows());
            }
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
    Q_UNUSED(selected)

    if(!mUiBlocked)
    {
        if(auto selectionModel = ui->tMegaFolders->selectionModel())
        {
            checkOkButton(selectionModel->selectedRows());
        }
    }
}

void NodeSelectorTreeViewWidget::checkOkButton(const QModelIndexList &selected)
{
    ui->bOk->setEnabled(mSelectType->okButtonEnabled(selected));
}

void NodeSelectorTreeViewWidget::onRenameClicked()
{
    auto node = std::unique_ptr<MegaNode>(mMegaApi->getNodeByHandle(getSelectedNodeHandle()));
    int access = mMegaApi->getAccess(node.get());
    //This is for an extra protection as we don´t show the rename action if one of this conditions are not met
    if (!node || access < MegaShare::ACCESS_FULL  || !node->isNodeKeyDecrypted())
    {
        return;
    }

    QPointer<RenameRemoteNodeDialog> dialog(new RenameRemoteNodeDialog(std::move(node), this));
    dialog->init();
    DialogOpener::showDialog(dialog, [this, dialog]
    {
        if(dialog->result() == QDialog::Accepted)
        {
            auto selectedIndex = getSelectedIndex();
            if(selectedIndex.isValid())
            {
                auto item = qvariant_cast<NodeSelectorModelItem*>(selectedIndex.data(toInt(NodeSelectorModelRoles::MODEL_ITEM_ROLE)));
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
    //This is for an extra protection as we don´t show the rename action if one of this conditions are not met
    if (!node || access < MegaShare::ACCESS_FULL || !node->isNodeKeyDecrypted())
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
        //Double protection in case the node properties changed while the node is deleted
        if (access == MegaShare::ACCESS_FULL
                && node->isNodeKeyDecrypted())
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

void NodeSelectorTreeViewWidget::setSelectedNodeHandle(const MegaHandle& selectedHandle, bool goToInit)
{
    if(selectedHandle == INVALID_HANDLE)
    {
        return;
    }

    auto node = std::shared_ptr<MegaNode>(mMegaApi->getNodeByHandle(selectedHandle));
    if (!node)
        return;

    if(goToInit)
    {
        mNavigationInfo.clear();
        setRootIndex(QModelIndex());
        checkBackForwardButtons();
    }

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

    onRootIndexChanged(node_column_idx);

    if(!node_column_idx.isValid())
    {
        setTitleText(getRootText());

        QModelIndexList selectedIndexes = ui->tMegaFolders->selectionModel()->selectedIndexes();
        foreach(auto& selection, selectedIndexes)
        {
            ui->tMegaFolders->selectionModel()->select(selection, QItemSelectionModel::ClearAndSelect | QItemSelectionModel::Rows);
        }
        ui->lOwnerIcon->setPixmap(QPixmap());
        ui->avatarSpacer->spacerItem()->changeSize(0, 0);
        ui->lIcon->setPixmap(QPixmap());
        ui->syncSpacer->spacerItem()->changeSize(0, 0);
        return;
    }

    //Taking the sync icon
    auto status_column_idx = proxy_idx.sibling(proxy_idx.row(), NodeSelectorModel::COLUMN::STATUS);
    QIcon syncIcon = qvariant_cast<QIcon>(status_column_idx.data(Qt::DecorationRole));


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

    auto item = qvariant_cast<NodeSelectorModelItem*>(node_column_idx.data(toInt(NodeSelectorModelRoles::MODEL_ITEM_ROLE)));
    if(!item)
    {
        return;
    }

    auto node = item->getNode();
    if(node)
    {
        setTitleText(MegaNodeNames::getNodeName(node.get()));
    }
}

QIcon NodeSelectorTreeViewWidget::getEmptyIcon()
{
    return QIcon();
}

QModelIndex NodeSelectorTreeViewWidget::getParentIncomingShareByIndex(QModelIndex idx)
{
    while(idx.isValid())
    {        
        if(NodeSelectorModelItem *item = qvariant_cast<NodeSelectorModelItem*>(idx.data(toInt(NodeSelectorModelRoles::MODEL_ITEM_ROLE))))
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

void NodeSelectorTreeViewWidget::Navigation::clear()
{
    backwardHandles.clear();
    forwardHandles.clear();
}

bool SelectType::isAllowedToNavigateInside(const QModelIndex &index)
{
    auto item = qvariant_cast<NodeSelectorModelItem*>(index.data(toInt(NodeSelectorModelRoles::MODEL_ITEM_ROLE)));
    if(!item)
    {
        return false;
    }
    return !(item->getNode()->isFile() || item->isCloudDrive());
}

void DownloadType::init(NodeSelectorTreeViewWidget *wdg)
{
    wdg->ui->bNewFolder->hide();
    wdg->ui->tMegaFolders->setSelectionMode(QAbstractItemView::ExtendedSelection);
    wdg->mModel->showFiles(true);
    wdg->mModel->showReadOnlyFolders(true);
}

bool DownloadType::okButtonEnabled(const QModelIndexList &selected)
{
    return !selected.isEmpty();
}

NodeSelectorModelItemSearch::Types DownloadType::allowedTypes()
{
    return NodeSelectorModelItemSearch::Type::CLOUD_DRIVE
            | NodeSelectorModelItemSearch::Type::INCOMING_SHARE
            | NodeSelectorModelItemSearch::Type::BACKUP;
}

void SyncType::init(NodeSelectorTreeViewWidget *wdg)
{
    wdg->mModel->setSyncSetupMode(true);
    wdg->ui->bNewFolder->setVisible(wdg->newFolderBtnCanBeVisisble());
    wdg->mModel->showFiles(false);
    wdg->mModel->showReadOnlyFolders(false);
}

void SyncType::newFolderButtonVisibility(NodeSelectorTreeViewWidget *wdg)
{
    if(wdg->newFolderBtnCanBeVisisble())
    {
        auto sourceIndex = wdg->mProxyModel->getIndexFromSource(wdg->ui->tMegaFolders->rootIndex());
        wdg->ui->bNewFolder->setVisible(sourceIndex.isValid() || wdg->newFolderBtnVisibleInRoot());
    }
}

bool SyncType::okButtonEnabled(const QModelIndexList &selected)
{
    bool enable(false);
    if(!selected.isEmpty() && selected.size() < 2)
    {
        auto& index = selected.at(0);
        bool isSyncable = index.data(toInt(NodeSelectorModelRoles::IS_SYNCABLE_FOLDER_ROLE)).toBool();
        bool isFile = index.data(toInt(NodeSelectorModelRoles::IS_FILE_ROLE)).toBool();
        if(isSyncable && !isFile)
        {
            enable = true;
        }
    }
    return enable;
}

NodeSelectorModelItemSearch::Types SyncType::allowedTypes()
{
    return NodeSelectorModelItemSearch::Type::CLOUD_DRIVE | NodeSelectorModelItemSearch::Type::INCOMING_SHARE;
}

bool SyncType::isAllowedToNavigateInside(const QModelIndex& index)
{
    if(!SelectType::isAllowedToNavigateInside(index))
    {
        return false;
    }
    auto item = qvariant_cast<NodeSelectorModelItem*>(index.data(toInt(NodeSelectorModelRoles::MODEL_ITEM_ROLE)));
    return !(item->getStatus() == NodeSelectorModelItem::Status::SYNC || item->getStatus() == NodeSelectorModelItem::Status::SYNC_CHILD);
}

void StreamType::init(NodeSelectorTreeViewWidget *wdg)
{
    wdg->ui->bNewFolder->hide();
    wdg->mModel->showFiles(true);
    wdg->mModel->showReadOnlyFolders(true);
}

bool StreamType::okButtonEnabled(const QModelIndexList &selected)
{
    bool enable(false);
    if(!selected.isEmpty() && selected.size() < 2)
    {
        auto& index = selected.at(0);
        enable = index.data(toInt(NodeSelectorModelRoles::IS_FILE_ROLE)).toBool();
    }
    return enable;
}

NodeSelectorModelItemSearch::Types StreamType::allowedTypes()
{
    return NodeSelectorModelItemSearch::Type::CLOUD_DRIVE
            | NodeSelectorModelItemSearch::Type::INCOMING_SHARE
            | NodeSelectorModelItemSearch::Type::BACKUP;
}

void UploadType::init(NodeSelectorTreeViewWidget *wdg)
{
    wdg->ui->bNewFolder->setVisible(wdg->newFolderBtnCanBeVisisble());
    wdg->mModel->showFiles(false);
    wdg->mModel->showReadOnlyFolders(false);
}

void UploadType::newFolderButtonVisibility(NodeSelectorTreeViewWidget *wdg)
{
    if(wdg->newFolderBtnCanBeVisisble())
    {
        auto sourceIndex = wdg->mProxyModel->getIndexFromSource(wdg->ui->tMegaFolders->rootIndex());
        wdg->ui->bNewFolder->setVisible(sourceIndex.isValid() || wdg->newFolderBtnVisibleInRoot());
    }
}

bool UploadType::okButtonEnabled(const QModelIndexList &selected)
{
    return !selected.isEmpty();
}

NodeSelectorModelItemSearch::Types UploadType::allowedTypes()
{
    return NodeSelectorModelItemSearch::Type::CLOUD_DRIVE
            | NodeSelectorModelItemSearch::Type::INCOMING_SHARE;
}
