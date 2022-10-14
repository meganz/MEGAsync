#include "MegaItemTreeView.h"
#include "MegaItem.h"
#include "MegaApplication.h"
#include "MegaItemProxyModel.h"
#include "MegaItemModel.h"

#include <QPainter>

MegaItemTreeView::MegaItemTreeView(QWidget* parent) :
    QTreeView(parent),
    mMegaApi(MegaSyncApp->getMegaApi())
{
    installEventFilter(this);
}

QModelIndex MegaItemTreeView::getIndexFromSourceModel(const QModelIndex& index) const
{
    if(!index.isValid())
    {
        return QModelIndex();
    }
    return proxyModel()->getIndexFromSource(index);
}

MegaItemProxyModel *MegaItemTreeView::proxyModel() const
{
    return static_cast<MegaItemProxyModel*>(model());
}

//Only used for single selection mode
MegaHandle MegaItemTreeView::getSelectedNodeHandle()
{
    MegaHandle ret = INVALID_HANDLE;
    if(selectionModel()->selectedRows().size() == 1)
    {
        if(auto node = proxyModel()->getNode(selectionModel()->selectedRows().first()))
            ret = node->getHandle();
    }
    return ret;
}

void MegaItemTreeView::verticalScrollbarValueChanged(int value)
{
//    if (verticalScrollBar()->maximum() / 2 < value)
//    {
//        value = verticalScrollBar()->maximum();
//        QTreeView::verticalScrollbarValueChanged(value);
//    }
    QTreeView::verticalScrollbarValueChanged(value);
}

bool MegaItemTreeView::eventFilter(QObject *obj, QEvent *evnt)
{
   // qDebug()<<evnt->type();
    return QTreeView::eventFilter(obj, evnt);
}

bool MegaItemTreeView::viewportEvent(QEvent *event)
{
    if(signalsBlocked())
    {
        return true;
    }
    return QTreeView::viewportEvent(event);
}

void MegaItemTreeView::drawBranches(QPainter *painter, const QRect &rect, const QModelIndex &index) const
{
    QModelIndex idx = getIndexFromSourceModel(index);
    MegaItem *item = static_cast<MegaItem*>(idx.internalPointer());
    if(item && item->isRoot())
    {
        QStyleOptionViewItem opt = viewOptions();
        opt.rect = rect;
        if(selectionModel()->isSelected(index))
        {
            opt.state |= QStyle::State_Selected;
        }
        style()->drawPrimitive(QStyle::PE_IndicatorBranch, &opt, painter, this);
        return;
    }
    QTreeView::drawBranches(painter, rect, index);
}

void MegaItemTreeView::mousePressEvent(QMouseEvent *event)
{
    QPoint pos = event->pos();
    QModelIndex index = getIndexFromSourceModel(indexAt(pos));
    MegaItem *item = static_cast<MegaItem*>(index.internalPointer());
    if(item && item->isRoot())
    {   //this line avoid to cloud drive being collapsed and at same time it allows to select it.
        QAbstractItemView::mousePressEvent(event);
    }
    else
    {
        QTreeView::mousePressEvent(event);
    }
}

void MegaItemTreeView::mouseDoubleClickEvent(QMouseEvent *event)
{
    if(event->button() != Qt::RightButton)
    {
        QTreeView::mouseDoubleClickEvent(event);
    }
}

void MegaItemTreeView::keyPressEvent(QKeyEvent *event)
{
    QModelIndexList selectedRows = selectionModel()->selectedRows();

    auto proxyModel = dynamic_cast<MegaItemProxyModel*>(model());

    static QModelIndex rootIndex = proxyModel->getIndexFromNode(MegaSyncApp->getRootNode());
    static QList<int> bannedFromRootKeyList = QList<int>() << Qt::Key_Left << Qt::Key_Right
                                                     << Qt::Key_Plus << Qt::Key_Minus;

    if(!bannedFromRootKeyList.contains(event->key()) || !selectedRows.contains(rootIndex))
    {
        QTreeView::keyPressEvent(event);
    }
}

void MegaItemTreeView::contextMenuEvent(QContextMenuEvent *event)
{
        if(selectionModel()->selectedRows().size() > 1)
            return;

        if(!indexAt(event->pos()).isValid())
        {
            return;
        }

        QMenu customMenu;
        auto node = std::unique_ptr<MegaNode>(mMegaApi->getNodeByHandle(getSelectedNodeHandle()));
        auto parent = std::unique_ptr<MegaNode>(mMegaApi->getParentNode(node.get()));

        if (parent && node)
        {
            int access = mMegaApi->getAccess(node.get());

            if (access == MegaShare::ACCESS_OWNER)
            {
                customMenu.addAction(tr("Get MEGA link"), this, SLOT(getMegaLink()));
            }

            if (access >= MegaShare::ACCESS_FULL)
            {
                customMenu.addAction(tr("Delete"), this, SLOT(removeNode()));
            }
        }

        if (!customMenu.actions().isEmpty())
            customMenu.exec(mapToGlobal(event->pos()));
}

void MegaItemTreeView::removeNode()
{
    emit removeNodeClicked();
}

void MegaItemTreeView::getMegaLink()
{
    emit getMegaLinkClicked();
}

MegaItemHeaderView::MegaItemHeaderView(Qt::Orientation orientation, QWidget *parent) :
    QHeaderView(orientation, parent)
{
    setDefaultAlignment(Qt::AlignLeft);
    setStretchLastSection(true);
    setDefaultSectionSize(35);
}

void MegaItemHeaderView::paintSection(QPainter *painter, const QRect &rect, int logicalIndex) const
{
    QRect vrect = rect; 

#ifdef _WIN32
    if(logicalIndex == MegaItemModel::USER)
        vrect.moveTo(vrect.x() - 2,vrect.y());
#endif

    QHeaderView::paintSection(painter, vrect, logicalIndex);
}


