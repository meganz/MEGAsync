#include "MegaItemTreeView.h"
#include "MegaItem.h"
#include "MegaApplication.h"
#include "MegaItemProxyModel.h"
#include "Platform.h"

#include <QPainter>
#include <QMenu>

MegaItemTreeView::MegaItemTreeView(QWidget* parent) :
    QTreeView(parent),
    mMegaApi(MegaSyncApp->getMegaApi())
{
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
        Platform::initMenu(&customMenu);
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
