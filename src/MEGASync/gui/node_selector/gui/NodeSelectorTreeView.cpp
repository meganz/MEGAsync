#include "NodeSelectorTreeView.h"
#include "../model/NodeSelectorModelItem.h"
#include "MegaApplication.h"
#include "../model/NodeSelectorProxyModel.h"
#include "Platform.h"
#include "../model/NodeSelectorModel.h"

#include <QPainter>
#include <QMenu>

NodeSelectorTreeView::NodeSelectorTreeView(QWidget* parent) :
    QTreeView(parent),
    //mIndexToEnter(QModelIndex()),
    mMegaApi(MegaSyncApp->getMegaApi())
{
    installEventFilter(this);
}

QModelIndex NodeSelectorTreeView::getIndexFromSourceModel(const QModelIndex& index) const
{
    if(!index.isValid())
    {
        return QModelIndex();
    }
    return proxyModel()->getIndexFromSource(index);
}

NodeSelectorProxyModel *NodeSelectorTreeView::proxyModel() const
{
    return static_cast<NodeSelectorProxyModel*>(model());
}

//Only used for single selection mode
MegaHandle NodeSelectorTreeView::getSelectedNodeHandle()
{
    MegaHandle ret = INVALID_HANDLE;
    if(selectionModel()->selectedRows().size() == 1)
    {
        if(auto node = proxyModel()->getNode(selectionModel()->selectedRows().first()))
            ret = node->getHandle();
    }
    return ret;
}

void NodeSelectorTreeView::setModel(QAbstractItemModel *model)
{
    QTreeView::setModel(model);
    connect(proxyModel(), &NodeSelectorProxyModel::navigateReady, this, &NodeSelectorTreeView::onNavigateReady);
}

void NodeSelectorTreeView::verticalScrollbarValueChanged(int value)
{
//    if (verticalScrollBar()->maximum() / 2 < value)
//    {
//        value = verticalScrollBar()->maximum();
//        QTreeView::verticalScrollbarValueChanged(value);
//    }
    QTreeView::verticalScrollbarValueChanged(value);
}

bool NodeSelectorTreeView::eventFilter(QObject *obj, QEvent *evnt)
{
    return QTreeView::eventFilter(obj, evnt);
}

bool NodeSelectorTreeView::viewportEvent(QEvent *event)
{
    if(signalsBlocked())
    {
        return true;
    }
    return QTreeView::viewportEvent(event);
}

void NodeSelectorTreeView::drawBranches(QPainter *painter, const QRect &rect, const QModelIndex &index) const
{
    QModelIndex idx = getIndexFromSourceModel(index);
    NodeSelectorModelItem *item = static_cast<NodeSelectorModelItem*>(idx.internalPointer());
    if(item && (item->isRoot() || item->isVault()))
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

void NodeSelectorTreeView::mousePressEvent(QMouseEvent *event)
{
    bool accept = true;
    if (style()->styleHint(QStyle::SH_ListViewExpand_SelectMouseType, 0, this) == QEvent::MouseButtonPress)
    {
        accept = mousePressorReleaseEvent(event);
    }

    if(accept)
    {
        QTreeView::mousePressEvent(event);
    }
}


void NodeSelectorTreeView::mouseReleaseEvent(QMouseEvent *event)
{
    bool accept = true;

    if (style()->styleHint(QStyle::SH_ListViewExpand_SelectMouseType, 0, this) == QEvent::MouseButtonRelease)
    {
        accept = mousePressorReleaseEvent(event);
    }
    if(accept)
    {
        QTreeView::mouseReleaseEvent(event);
    }
}

void NodeSelectorTreeView::mouseDoubleClickEvent(QMouseEvent *event)
{
    if(event->button() != Qt::RightButton)
    {
        QModelIndex clickedIndex = indexAt(event->pos());
        if(clickedIndex.isValid())
        {
            auto sourceIndexToEnter = proxyModel()->mapToSource(clickedIndex);
            if(proxyModel()->sourceModel()->canFetchMore(sourceIndexToEnter))
            {
                proxyModel()->setExpandMapped(false);
                proxyModel()->sourceModel()->fetchMore(sourceIndexToEnter);
                return;
            }
        }
        QTreeView::mouseDoubleClickEvent(event);
    }
}

void NodeSelectorTreeView::keyPressEvent(QKeyEvent *event)
{
    QModelIndexList selectedRows = selectionModel()->selectedRows();

    static QModelIndex rootIndex = proxyModel()->getIndexFromNode(MegaSyncApp->getRootNode());
    static QList<int> bannedFromRootKeyList = QList<int>() << Qt::Key_Left << Qt::Key_Right
                                                     << Qt::Key_Plus << Qt::Key_Minus;

    if(!bannedFromRootKeyList.contains(event->key()) || !selectedRows.contains(rootIndex))
    {
        if(event->key() == Qt::Key_F2)
        {
            renameNode();
        }

        QTreeView::keyPressEvent(event);
    }
}

void NodeSelectorTreeView::contextMenuEvent(QContextMenuEvent *event)
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
        auto proxyModel = static_cast<NodeSelectorProxyModel*>(model());
        if (parent && node)
        {
            int access = mMegaApi->getAccess(node.get());

            if (access == MegaShare::ACCESS_OWNER)
            {
                customMenu.addAction(tr("Get MEGA link"), this, SLOT(getMegaLink()));
            }

            if (access >= MegaShare::ACCESS_FULL && proxyModel->canBeDeleted())
            {
                customMenu.addAction(tr("Rename"), this, SLOT(renameNode()));
                customMenu.addAction(tr("Delete"), this, SLOT(removeNode()));
            }
        }

        if (!customMenu.actions().isEmpty())
            customMenu.exec(mapToGlobal(event->pos()));
}

void NodeSelectorTreeView::removeNode()
{
    emit removeNodeClicked();
}

void NodeSelectorTreeView::renameNode()
{
    emit renameNodeClicked();
}

void NodeSelectorTreeView::getMegaLink()
{
    emit getMegaLinkClicked();
}

void NodeSelectorTreeView::onNavigateReady(const QModelIndex &index)
{
    if(index.isValid())
    {
        //Loading finished
        emit proxyModel()->getMegaModel()->blockUi(false);

        QPoint point = visualRect(index).center();
        QMouseEvent mouseEvent(QEvent::MouseButtonDblClick, point, Qt::LeftButton, Qt::LeftButton, Qt::NoModifier);
        mouseDoubleClickEvent(&mouseEvent);
    }
}

bool NodeSelectorTreeView::mousePressorReleaseEvent(QMouseEvent *event)
{
    QPoint pos = event->pos();
    QModelIndex index = getIndexFromSourceModel(indexAt(pos));
    NodeSelectorModelItem *item = static_cast<NodeSelectorModelItem*>(index.internalPointer());
    if(item && item->isRoot())
    {   //this line avoid to cloud drive being collapsed and at same time it allows to select it.
        if(event->type() == QMouseEvent::MouseButtonPress)
        {
            QAbstractItemView::mousePressEvent(event);
        }
        else if(event->type() == QMouseEvent::MouseButtonRelease)
        {
            QAbstractItemView::mouseReleaseEvent(event);
        }
        return false;
    }
    else
    {
        QModelIndex clickedIndex = indexAt(event->pos());
        if(clickedIndex.isValid() && !clickedIndex.data(toInt(NodeRowDelegateRoles::INIT_ROLE)).toBool())
        {
            int position = columnViewportPosition(0);
            int height = rowHeight(clickedIndex);
            int level = 0;
            QModelIndex idx = clickedIndex;
            idx = idx.parent();
            while(rootIndex() != idx)
            {
                level++;
                idx = idx.parent();
            }
            int identation = indentation() * level;
            QRect rect(position + identation, event->pos().y(), indentation(), height);

            if(rect.contains(event->pos()))
            {
                if(!isExpanded(clickedIndex))
                {
                    auto sourceIndexToExpand = proxyModel()->mapToSource(clickedIndex);
                    if(proxyModel()->sourceModel()->canFetchMore(sourceIndexToExpand))
                    {
                        proxyModel()->setExpandMapped(true);
                        proxyModel()->sourceModel()->fetchMore(sourceIndexToExpand);
                    }

                    if(event->type() == QMouseEvent::MouseButtonPress)
                    {
                        QAbstractItemView::mousePressEvent(event);
                    }
                    else if(event->type() == QMouseEvent::MouseButtonRelease)
                    {
                        QAbstractItemView::mouseReleaseEvent(event);
                    }
                    return false;
                }
            }

        }
    }
    return true;
}

NodSelectorTreeViewHeaderView::NodSelectorTreeViewHeaderView(Qt::Orientation orientation, QWidget *parent) :
    QHeaderView(orientation, parent)
{
    setDefaultAlignment(Qt::AlignLeft);
    setStretchLastSection(true);
    setDefaultSectionSize(35);
}

void NodSelectorTreeViewHeaderView::paintSection(QPainter *painter, const QRect &rect, int logicalIndex) const
{
    QRect vrect = rect; 

#ifdef _WIN32
    if(logicalIndex == NodeSelectorModel::USER)
        vrect.moveTo(vrect.x() - 2,vrect.y());
#endif

    QHeaderView::paintSection(painter, vrect, logicalIndex);
}


