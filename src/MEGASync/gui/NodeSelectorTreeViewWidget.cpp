#include "NodeSelectorTreeViewWidget.h"
#include "ui_NodeSelectorTreeViewWidget.h"
#include "MegaItemModel.h"
#include "MegaItemDelegates.h"
#include "MegaApplication.h"


NodeSelectorTreeViewWidget::NodeSelectorTreeViewWidget(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::NodeSelectorTreeViewWidget),
    mMegaApi(MegaSyncApp->getMegaApi())
{
    ui->setupUi(this);

    ui->tMegaFolders->setContextMenuPolicy(Qt::DefaultContextMenu);
    ui->tMegaFolders->setExpandsOnDoubleClick(false);
    ui->tMegaFolders->setSortingEnabled(true);
    ui->tMegaFolders->setHeader(new MegaItemHeaderView(Qt::Horizontal));
    ui->tMegaFolders->header()->setFixedHeight(MegaItemModel::ROW_HEIGHT);
    ui->tMegaFolders->header()->moveSection(MegaItemModel::STATUS, MegaItemModel::NODE);
    ui->tMegaFolders->header()->setProperty("HeaderIconCenter", true);
    ui->tMegaFolders->setColumnWidth(MegaItemModel::COLUMN::STATUS, MegaItemModel::ROW_HEIGHT * 2);
    ui->tMegaFolders->setItemDelegate(new  NodeRowDelegate(ui->tMegaFolders));
    ui->tMegaFolders->setItemDelegateForColumn(MegaItemModel::STATUS, new IconDelegate(ui->tMegaFolders));
    ui->tMegaFolders->setItemDelegateForColumn(MegaItemModel::USER, new IconDelegate(ui->tMegaFolders));
    //ui->tMegaFolders->setExpanded(mProxyModel->getIndexFromHandle(MegaSyncApp->getRootNode()->getHandle()), true);
    ui->tMegaFolders->setTextElideMode(Qt::ElideMiddle);
    ui->tMegaFolders->sortByColumn(MegaItemModel::NODE, Qt::AscendingOrder);
    ui->lFolderName->setText(tr("Cloud drive"));


    connect(ui->tMegaFolders->selectionModel(), &QItemSelectionModel::selectionChanged, this, &NodeSelectorTreeViewWidget::onSelectionChanged);
    connect(ui->tMegaFolders, &MegaItemTreeView::removeNodeClicked, this, &NodeSelectorTreeViewWidget::onDeleteClicked);
    connect(ui->tMegaFolders, &MegaItemTreeView::getMegaLinkClicked, this, &NodeSelectorTreeViewWidget::onGenMEGALinkClicked);
    connect(ui->tMegaFolders, &QTreeView::doubleClicked, this, &NodeSelectorTreeViewWidget::onItemDoubleClick);
    connect(ui->bForward, &QPushButton::clicked, this, &NodeSelectorTreeViewWidget::onGoForwardClicked);
    connect(ui->bBack, &QPushButton::clicked, this, &NodeSelectorTreeViewWidget::onGoBackClicked);
    connect(ui->tMegaFolders->header(), &QHeaderView::sectionResized, this, &NodeSelectorTreeViewWidget::onSectionResized);

}

void NodeSelectorTreeViewWidget::onSectionResized()
{
    if(!mManuallyResizedColumn
            && ui->tMegaFolders->header()->rect().contains(ui->tMegaFolders->mapFromGlobal(QCursor::pos())))
    {
        mManuallyResizedColumn = true;
    }
}

void NodeSelectorTreeViewWidget::onGoBackClicked()
{
    QModelIndex indexToGo;
    if(isCloudDrive())
    {
        mNavCloudDrive.appendToForward(getHandleByIndex(ui->tMegaFolders->rootIndex()));
        indexToGo = getIndexFromHandle(mNavCloudDrive.backwardHandles.last());
        mNavCloudDrive.backwardHandles.removeLast();
    }
    else
    {
        mNavInShares.appendToForward(getHandleByIndex(ui->tMegaFolders->rootIndex()));
        indexToGo = getIndexFromHandle(mNavInShares.backwardHandles.last());
        mNavInShares.backwardHandles.removeLast();
    }
    setRootIndex(indexToGo);
    checkBackForwardButtons();
    checkNewFolderButtonVisibility();
}

void NodeSelectorTreeViewWidget::onGoForwardClicked()
{
    QModelIndex indexToGo;
    if(isCloudDrive())
    {
        mNavCloudDrive.appendToBackward(getHandleByIndex(ui->tMegaFolders->rootIndex()));
        indexToGo = getIndexFromHandle(mNavCloudDrive.forwardHandles.last());
        mNavCloudDrive.forwardHandles.removeLast();
    }
    else
    {
        mNavInShares.appendToBackward(getHandleByIndex(ui->tMegaFolders->rootIndex()));
        indexToGo = getIndexFromHandle(mNavInShares.forwardHandles.last());
        mNavInShares.forwardHandles.removeLast();
    }
    setRootIndex(indexToGo);
    checkBackForwardButtons();
    checkNewFolderButtonVisibility();
}

void NodeSelectorTreeViewWidget::onItemDoubleClick(const QModelIndex &index)
{
    if(!isAllowedToEnterInIndex(index) )
        return;

    if(isCloudDrive())
    {
        mNavCloudDrive.appendToBackward(getHandleByIndex(ui->tMegaFolders->rootIndex()));
        mNavCloudDrive.removeFromForward(mProxyModel->getHandle(index));
    }
    else
    {
        mNavInShares.appendToBackward(getHandleByIndex(ui->tMegaFolders->rootIndex()));
        mNavInShares.removeFromForward(mProxyModel->getHandle(index));
    }

    setRootIndex(index);
    checkBackForwardButtons();
    checkNewFolderButtonVisibility();
}

void NodeSelectorTreeViewWidget::onSelectionChanged(const QItemSelection& selected, const QItemSelection& deselected)
{
    Q_UNUSED(deselected)
    if(mSelectMode == UPLOAD_SELECT || mSelectMode == DOWNLOAD_SELECT)
    {
        //ui->bOk->setEnabled(true);
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
                ui->bOk->setEnabled(item->getNode()->isFile());
            }
            else if(mSelectMode == NodeSelector::SYNC_SELECT)
            {
                ui->bOk->setEnabled(item->isSyncable());
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

    QPointer<NodeSelector> currentDialog = this;
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
