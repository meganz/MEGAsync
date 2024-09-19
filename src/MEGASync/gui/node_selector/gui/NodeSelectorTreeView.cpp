#include "NodeSelectorTreeView.h"

#include "MegaApplication.h"
#include "NodeSelectorModel.h"
#include "NodeSelectorModelItem.h"
#include "NodeSelectorProxyModel.h"
#include "Platform.h"

#include <QMenu>
#include <QMouseEvent>
#include <QPainter>

NodeSelectorTreeView::NodeSelectorTreeView(QWidget* parent) :
    LoadingSceneView<NodeSelectorLoadingDelegate, QTreeView>(parent),
    mMegaApi(MegaSyncApp->getMegaApi())
{
    installEventFilter(this);
    loadingView().setDelayTimeToShowInMs(150);
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
    if(!selectionModel())
    {
        return ret;
    }

    if(selectionModel()->selectedRows().size() == 1)
    {
        if(auto node = proxyModel()->getNode(selectionModel()->selectedRows().first()))
            ret = node->getHandle();
    }
    return ret;
}

QList<MegaHandle> NodeSelectorTreeView::getMultiSelectionNodeHandle() const
{
    QList<MegaHandle> ret;

    if(!selectionModel())
    {
        return ret;
    }

    auto selectedRows = selectionModel()->selectedRows();

    //If there is no selection, add the root index
    if(selectedRows.isEmpty())
    {
        auto index(rootIndex());
        if(index.isValid())
        {
            auto item = proxyModel()->getMegaModel()->getItemByIndex(index);
            if(item)
            {
                ret.append(item->getNode()->getHandle());
            }
        }
    }
    else
    {
        foreach(auto& s_index, selectedRows)
        {
            if(auto node = proxyModel()->getNode(s_index))
                ret.append(node->getHandle());
        }
    }

    return ret;
}

void NodeSelectorTreeView::setModel(QAbstractItemModel *model)
{
    QTreeView::setModel(model);
    connect(proxyModel(), &NodeSelectorProxyModel::navigateReady, this, &NodeSelectorTreeView::onNavigateReady);
}

void NodeSelectorTreeView::drawBranches(QPainter *painter, const QRect &rect, const QModelIndex &index) const
{
    auto item = qvariant_cast<NodeSelectorModelItem*>(index.data(toInt(NodeSelectorModelRoles::MODEL_ITEM_ROLE)));
    if(item && (item->isCloudDrive() || item->isVault()))
    {
        QStyleOptionViewItem opt = viewOptions();
        opt.rect = rect;
        if(!selectionModel())
        {
            return;
        }
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
    if(!selectionModel())
    {
        return;
    }

    QModelIndexList selectedRows = selectionModel()->selectedRows();

    static QModelIndex cdRootIndex = proxyModel()->getIndexFromNode(MegaSyncApp->getRootNode());
    static QList<int> bannedFromRootKeyList = QList<int>() << Qt::Key_Left << Qt::Key_Right
                                                     << Qt::Key_Plus << Qt::Key_Minus;

    if(!bannedFromRootKeyList.contains(event->key()) || !selectedRows.contains(cdRootIndex))
    {
        if(event->key() == Qt::Key_F2)
        {
            renameNode();
        }
        else if(event->key() == Qt::Key_Enter || event->key() == Qt::Key_Return)
        {
            if(!selectedRows.isEmpty())
            {
                if(selectedRows.first() == rootIndex() || selectedRows.size() > 1)
                {
                    emit nodeSelected();
                }
                else
                {
                    auto node = std::unique_ptr<MegaNode>(mMegaApi->getNodeByHandle(getSelectedNodeHandle()));
                    if(node)
                    {
                        if(node->isFolder())
                        {
                            emit doubleClicked(selectedRows.first());
                        }
                        else
                        {
                            emit nodeSelected();
                        }
                    }
                }
            }
        }
        else if(event->key() == Qt::Key_Delete)
        {
            auto handlesToRemove(getMultiSelectionNodeHandle());
            if (getSourceModel()->areAllNodesEligibleForDeletion(handlesToRemove))
            {
                removeNode();
            }
        }

        QTreeView::keyPressEvent(event);
    }
}

void NodeSelectorTreeView::contextMenuEvent(QContextMenuEvent *event)
{
    QMenu customMenu;
    Platform::getInstance()->initMenu(&customMenu, "CustomMenu");

    if(!selectionModel())
    {
        return;
    }

    if(!indexAt(event->pos()).isValid())
    {
        return;
    }

    auto proxyModel = static_cast<NodeSelectorProxyModel*>(model());
    auto sourceModel = proxyModel->getMegaModel();

    auto selectedHandle(getSelectedNodeHandle());

    if(selectionModel()->selectedRows().size() == 1)
    {
        std::unique_ptr<mega::MegaNode> node(
            MegaSyncApp->getMegaApi()->getNodeByHandle(selectedHandle));
        if (node)
        {
            int access = sourceModel->getNodeAccess(node.get());

            if (access != MegaShare::ACCESS_UNKNOWN)
            {
                if (access == MegaShare::ACCESS_OWNER)
                {
                    customMenu.addAction(tr("Get MEGA link"),
                                         this,
                                         &NodeSelectorTreeView::getMegaLink);
                }

                if (proxyModel->isNotAProtectedModel() && access >= MegaShare::ACCESS_FULL)
                {
                    customMenu.addAction(tr("Rename"), this, &NodeSelectorTreeView::renameNode);
                }
            }
        }
    }

    //All or none
    if (proxyModel->isNotAProtectedModel() &&
        sourceModel->areAllNodesEligibleForDeletion(getMultiSelectionNodeHandle()))
    {
        customMenu.addAction(
            tr("Delete"), this, [this]() { removeNode(); });
    }

    if (!customMenu.actions().isEmpty())
            customMenu.exec(mapToGlobal(event->pos()));
}

NodeSelectorModel* NodeSelectorTreeView::getSourceModel() const
{
    auto proxyModel = static_cast<NodeSelectorProxyModel*>(model());
    return proxyModel->getMegaModel();
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
    QModelIndex index = indexAt(pos);
    if(!index.isValid())
    {
        return false;
    }

    auto item = qvariant_cast<NodeSelectorModelItem*>(index.data(toInt(NodeSelectorModelRoles::MODEL_ITEM_ROLE)));
    if(item && item->isCloudDrive())
    {   //this line avoid to cloud drive being collapsed and at same time it allows to select it.
       return handleStandardMouseEvent(event);
    }
    else
    {
        if(!index.data(toInt(NodeRowDelegateRoles::INIT_ROLE)).toBool())
        {
            int position = columnViewportPosition(0);
            QModelIndex idx = index.parent();
            while(rootIndex() != idx)
            {
                position += indentation();
                idx = idx.parent();
            }
            QRect rect(position, event->pos().y(), indentation(), rowHeight(index));

            if(rect.contains(event->pos()))
            {
                if(!isExpanded(index))
                {
                    auto sourceIndexToExpand = proxyModel()->mapToSource(index);
                    if(proxyModel()->sourceModel()->canFetchMore(sourceIndexToExpand))
                    {
                        proxyModel()->setExpandMapped(true);
                        proxyModel()->sourceModel()->fetchMore(sourceIndexToExpand);
                    }

                    return handleStandardMouseEvent(event);
                }
            }
        }
    }
    return true;
}

bool NodeSelectorTreeView::handleStandardMouseEvent(QMouseEvent* event)
{
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

NodeSelectorTreeViewHeaderView::NodeSelectorTreeViewHeaderView(Qt::Orientation orientation, QWidget *parent) :
    QHeaderView(orientation, parent)
{
    setDefaultAlignment(Qt::AlignLeft);
    setStretchLastSection(true);
    setDefaultSectionSize(35);
}

void NodeSelectorTreeViewHeaderView::paintSection(QPainter *painter, const QRect &rect, int logicalIndex) const
{
    painter->save();
    QHeaderView::paintSection(painter, rect, logicalIndex);
    painter->restore();
    if(logicalIndex == NodeSelectorModel::USER || logicalIndex == NodeSelectorModel::STATUS)
    {  
        QRect iconRect(QPoint(rect.topLeft()), QSize(18, 18));
        iconRect.moveCenter(rect.center());
        QIcon icon = model()->headerData(logicalIndex, Qt::Orientation::Horizontal, toInt(HeaderRoles::ICON_ROLE)).value<QIcon>();
        if(!icon.isNull())
        {
            painter->setRenderHint(QPainter::Antialiasing, true);
            painter->setRenderHint(QPainter::SmoothPixmapTransform, true);
            icon.paint(painter, iconRect, Qt::AlignVCenter | Qt::AlignHCenter);
        }
    }
}

