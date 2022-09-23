#include "NodeSelectorTreeViewWidget.h"
#include "ui_NodeSelectorTreeViewWidget.h"
#include "MegaItemModel.h"
#include "MegaItemDelegates.h"
#include "MegaApplication.h"
#include "QMegaMessageBox.h"
#include "MegaItemProxyModel.h"
#include "MegaItemModel.h"
#include "mega/utils.h"

const char* NodeSelectorTreeViewWidget::IN_SHARES = "Incoming shares";
const char* NodeSelectorTreeViewWidget::CLD_DRIVE = "Cloud drive";

const int NodeSelectorTreeViewWidget::LABEL_ELIDE_MARGIN = 100;


NodeSelectorTreeViewWidget::NodeSelectorTreeViewWidget(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::NodeSelectorTreeViewWidget),
    mProxyModel(nullptr),
    mMegaApi(MegaSyncApp->getMegaApi()),
    mManuallyResizedColumn(false),
    mDelegateListener(mega::make_unique<QTMegaRequestListener>(mMegaApi, this)),
    mModel(nullptr)
{
    ui->setupUi(this);

    connect(ui->tMegaFolders->selectionModel(), &QItemSelectionModel::selectionChanged, this, &NodeSelectorTreeViewWidget::onSelectionChanged);
    connect(ui->tMegaFolders, &MegaItemTreeView::removeNodeClicked, this, &NodeSelectorTreeViewWidget::onDeleteClicked);
    connect(ui->tMegaFolders, &MegaItemTreeView::getMegaLinkClicked, this, &NodeSelectorTreeViewWidget::onGenMEGALinkClicked);
    connect(ui->tMegaFolders, &QTreeView::doubleClicked, this, &NodeSelectorTreeViewWidget::onItemDoubleClick);
    connect(ui->bForward, &QPushButton::clicked, this, &NodeSelectorTreeViewWidget::onGoForwardClicked);
    connect(ui->bBack, &QPushButton::clicked, this, &NodeSelectorTreeViewWidget::onGoBackClicked);
    connect(ui->tMegaFolders->header(), &QHeaderView::sectionResized, this, &NodeSelectorTreeViewWidget::onSectionResized);
    connect(ui->leSearch, &QLineEdit::textEdited, this, &NodeSelectorTreeViewWidget::onSearchBoxEdited);

    if(auto rootNode = std::unique_ptr<MegaNode>(mMegaApi->getRootNode()))
    {
        mNavigationInfo.expandedHandles.append(rootNode->getHandle());
    }

    checkBackForwardButtons();
    installEventFilter(this);
}

void NodeSelectorTreeViewWidget::changeEvent(QEvent *event)
{
    if (event->type() == QEvent::LanguageChange)
    {
        if(!ui->tMegaFolders->rootIndex().isValid())
        {
            //TODO SET CORRECT TEXT
            ui->lFolderName->setText(tr("CLD_DRIVE"));
        }
    }
    QWidget::changeEvent(event);
}

bool NodeSelectorTreeViewWidget::eventFilter(QObject *o, QEvent *e)
{
    if(o == this && e->type() == QEvent::ParentChange)
    {
        mProxyModel = std::unique_ptr<MegaItemProxyModel>(new MegaItemProxyModel(this));

        mModel = getModel();
        mModel->fillRootItems();
        mProxyModel->setSourceModel(mModel.get());
        QObject* parent_obj = parent();
        NodeSelector* nod = nullptr;

        do{
          nod = dynamic_cast<NodeSelector*>(parent_obj);
          parent_obj = parent_obj->parent();
        }
        while(nod == nullptr);

        mSelectMode = nod->getSelectMode();
        switch(mSelectMode)
        {
            case NodeSelector::SYNC_SELECT:
                mModel->setSyncSetupMode(true);
                // fall through
            case NodeSelector::UPLOAD_SELECT:
                mProxyModel->showReadOnlyFolders(false);
                mModel->showFiles(false);
                break;
            case NodeSelector::DOWNLOAD_SELECT:
                mModel->showFiles(true);
                ui->tMegaFolders->setSelectionMode(QAbstractItemView::ExtendedSelection);
                break;
            case NodeSelector::STREAM_SELECT:
                mModel->showFiles(true);
                setWindowTitle(tr("Select items"));
                break;
        }
        ui->tMegaFolders->setModel(mProxyModel.get());
        ui->tMegaFolders->setExpanded(mProxyModel->getIndexFromHandle(MegaSyncApp->getRootNode()->getHandle()), true);
        connect(mModel.get(), &MegaItemModel::modelReset, this, &NodeSelectorTreeViewWidget::onModelReset);

        ui->tMegaFolders->setContextMenuPolicy(Qt::DefaultContextMenu);
        ui->tMegaFolders->setExpandsOnDoubleClick(false);
        ui->tMegaFolders->setSortingEnabled(true);
        ui->tMegaFolders->setHeader(new MegaItemHeaderView(Qt::Horizontal));
        ui->tMegaFolders->header()->setFixedHeight(MegaItemModel::ROW_HEIGHT);
        ui->tMegaFolders->header()->moveSection(MegaItemModel::STATUS, MegaItemModel::NODE);
        ui->tMegaFolders->header()->setProperty("HeaderIconCenter", true);
        ui->tMegaFolders->setColumnWidth(MegaItemModel::COLUMN::STATUS, MegaItemModel::ROW_HEIGHT * 2);
        ui->tMegaFolders->setItemDelegate(new NodeRowDelegate(ui->tMegaFolders));
        ui->tMegaFolders->setItemDelegateForColumn(MegaItemModel::STATUS, new IconDelegate(ui->tMegaFolders));
        ui->tMegaFolders->setItemDelegateForColumn(MegaItemModel::USER, new IconDelegate(ui->tMegaFolders));
        ui->tMegaFolders->setTextElideMode(Qt::ElideMiddle);
        ui->tMegaFolders->sortByColumn(MegaItemModel::NODE, Qt::AscendingOrder);
        setRootIndex(QModelIndex());
    }
    return QWidget::eventFilter(o, e);
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
    ui->tMegaFolders->setColumnWidth(MegaItemModel::COLUMN::NODE, qRound(ui->tMegaFolders->width() * 0.57));
}

void NodeSelectorTreeViewWidget::resizeEvent(QResizeEvent *)
{
    if(!mManuallyResizedColumn)
    {
        ui->tMegaFolders->setColumnWidth(MegaItemModel::COLUMN::NODE, qRound(ui->tMegaFolders->width() * 0.57));
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

void NodeSelectorTreeViewWidget::onSearchBoxEdited(const QString &text)
{
    mProxyModel->setTextFilter(text);
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

void NodeSelectorTreeViewWidget::newFolderClicked()
{
    auto parentNode = mProxyModel->getNode(ui->tMegaFolders->rootIndex());
    if (!parentNode)
    {
        parentNode = MegaSyncApp->getRootNode();
        if (!parentNode)
            return;
    }

    NewFolderDialog dialog(parentNode, this);

    auto result = dialog.show();
    auto newNode = dialog.getNewNode();
    //IF the dialog return a node, there are two scenarios:
    //1) The dialog has been accepted, a new folder has been created
    //2) The dialog has been rejected because the folder already exists. If so, select the existing folder
    if(newNode)
    {
        mega::MegaHandle handle = newNode->getHandle();

        if(result == QDialog::Accepted)
        {
            QModelIndex idx = ui->tMegaFolders->rootIndex();
            if(!idx.isValid())
            {
                idx = mProxyModel->getIndexFromNode(MegaSyncApp->getRootNode());
            }
            mProxyModel->addNode(std::move(newNode), idx);
        }

        setSelectedNodeHandle(handle);
    }
}

bool NodeSelectorTreeViewWidget::isAllowedToEnterInIndex(const QModelIndex &idx)
{
    auto source_idx = mProxyModel->getIndexFromSource(idx);
    MegaItem *item = static_cast<MegaItem*>(source_idx.internalPointer());
    if(item)
    {
        if((item->getNode()->isFile())
           || (item->isRoot())
           || (mSelectMode == NodeSelector::SYNC_SELECT && (item->getStatus() == MegaItem::SYNC || item->getStatus() == MegaItem::SYNC_CHILD)))
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
        emit SetVisibleNewFolderBtn(sourceIndex.isValid() /*|| isCloudDrive()*/);
    }
}

void NodeSelectorTreeViewWidget::onSelectionChanged(const QItemSelection& selected, const QItemSelection& deselected)
{
    Q_UNUSED(deselected)
    if(mSelectMode == UPLOAD_SELECT || mSelectMode == DOWNLOAD_SELECT)
    {
        emit EnableOK(true);
        return;
    }
    foreach(auto& index, selected.indexes())
    {
        auto source_idx = mProxyModel->getIndexFromSource(index);
        MegaItem *item = static_cast<MegaItem*>(source_idx.internalPointer());
        if(item)
        {
            if(mSelectMode == NodeSelector::STREAM_SELECT)
            {
                emit EnableOK(item->getNode()->isFile());
            }
            else if(mSelectMode == NodeSelector::SYNC_SELECT)
            {
                emit EnableOK(item->getNode()->isFile());
            }
        }
    }
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
        emit EnableNewFolder(false);
        emit EnableOK(false);
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
    emit EnableNewFolder(true);
    emit EnableOK(true);

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
    auto node = std::shared_ptr<MegaNode>(mMegaApi->getNodeByHandle(selectedHandle));
    if (!node)
        return;

    QVector<QModelIndex> modelIndexList = mProxyModel->getRelatedModelIndexes(node/*, isInShare*/);

    if(modelIndexList.size() > 1)
    {
        foreach(QModelIndex idx, modelIndexList)
        {
            ui->tMegaFolders->expand(idx);
        }
    }

    if(modelIndexList.size() > 0)
    {
        ui->tMegaFolders->selectionModel()->setCurrentIndex(modelIndexList.last(), QItemSelectionModel::ClearAndSelect | QItemSelectionModel::Rows);
        ui->tMegaFolders->selectionModel()->select(modelIndexList.last(), QItemSelectionModel::ClearAndSelect | QItemSelectionModel::Rows);
    }
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
    auto node_column_idx = proxy_idx.sibling(proxy_idx.row(), MegaItemModel::COLUMN::NODE);

    ui->tMegaFolders->setRootIndex(node_column_idx);

    auto source_idx = mProxyModel->getIndexFromSource(node_column_idx);
    setRootIndex_Reimplementation(source_idx);

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
    auto status_column_idx = proxy_idx.sibling(proxy_idx.row(), MegaItemModel::COLUMN::STATUS);
    QIcon syncIcon = qvariant_cast<QIcon>(status_column_idx.data(Qt::DecorationRole));

    MegaItem *item = static_cast<MegaItem*>(source_idx.internalPointer());
    if(!item)
        return;

    if(!syncIcon.isNull())
    {
        QPixmap pm = syncIcon.pixmap(QSize(MegaItem::ICON_SIZE, MegaItem::ICON_SIZE), QIcon::Normal);
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
        if(MegaItem *item = static_cast<MegaItem*>(idx.internalPointer()))
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
    delete ui;
}

void NodeSelectorTreeViewWidget::Navigation::removeFromForward(const mega::MegaHandle &handle)
{
    if(forwardHandles.size() == 0)
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

NodeSelectorTreeViewWidgetCloudDrive::NodeSelectorTreeViewWidgetCloudDrive(QWidget *parent)
    : NodeSelectorTreeViewWidget(parent)
{
    setTitle(tr("Cloud drive"));
}

QString NodeSelectorTreeViewWidgetCloudDrive::getRootText()
{
    return tr(CLD_DRIVE);
}

void NodeSelectorTreeViewWidgetCloudDrive::onModelReset()
{
    ui->tMegaFolders->setExpanded(mProxyModel->index(0,0,QModelIndex()),true);
}

std::unique_ptr<MegaItemModel> NodeSelectorTreeViewWidgetCloudDrive::getModel()
{
    return std::unique_ptr<MegaItemModelCloudDrive>(new MegaItemModelCloudDrive);
}

void NodeSelectorTreeViewWidgetCloudDrive::setRootIndex_Reimplementation(const QModelIndex &source_idx)
{
    Q_UNUSED(source_idx)
    mProxyModel->showOwnerColumn(false);
}

NodeSelectorTreeViewWidgetIncomingShares::NodeSelectorTreeViewWidgetIncomingShares(QWidget *parent)
    : NodeSelectorTreeViewWidget(parent)
{
    setTitle(tr("Incoming shares"));
}

QString NodeSelectorTreeViewWidgetIncomingShares::getRootText()
{
    return tr(IN_SHARES);
}

std::unique_ptr<MegaItemModel> NodeSelectorTreeViewWidgetIncomingShares::getModel()
{
    return std::unique_ptr<MegaItemModelIncomingShares>(new MegaItemModelIncomingShares);
}

void NodeSelectorTreeViewWidgetIncomingShares::setRootIndex_Reimplementation(const QModelIndex &source_idx)
{
    if(source_idx.isValid())
    {
        mProxyModel->showOwnerColumn(false);
        QModelIndex in_share_idx = getParentIncomingShareByIndex(source_idx);
        in_share_idx = in_share_idx.sibling(in_share_idx.row(), MegaItemModel::COLUMN::USER);
        QPixmap pm = qvariant_cast<QPixmap>(in_share_idx.data(Qt::DecorationRole));
        QString tooltip = in_share_idx.data(Qt::ToolTipRole).toString();
        ui->lOwnerIcon->setToolTip(tooltip);
        ui->lOwnerIcon->setPixmap(pm);
        ui->avatarSpacer->spacerItem()->changeSize(10, 0);
    }
    else
    {
        mProxyModel->showOwnerColumn(true);
    }
}
