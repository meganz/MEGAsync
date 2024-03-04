#include "NodeSelectorTreeView.h"
#include "../model/NodeSelectorModelItem.h"
#include "MegaApplication.h"
#include "../model/NodeSelectorProxyModel.h"
#include "Platform.h"
#include "../model/NodeSelectorModel.h"

#include <QPainter>
#include <QMenu>

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

void NodeSelectorTreeView::setModel(QAbstractItemModel *model)
{
    QTreeView::setModel(model);
    connect(proxyModel(), &NodeSelectorProxyModel::navigateReady, this, &NodeSelectorTreeView::onNavigateReady);

#if QT_VERSION < QT_VERSION_CHECK(5, 15, 0)
    connect(selectionModel(), &QItemSelectionModel::currentRowChanged, this, &NodeSelectorTreeView::onCurrentRowChanged);
#endif
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
                            doubleClicked(selectedRows.first());
                        }
                        else
                        {
                            emit nodeSelected();
                        }
                    }
                }
            }
        }

        QTreeView::keyPressEvent(event);
    }
}

void NodeSelectorTreeView::contextMenuEvent(QContextMenuEvent *event)
{
        if(!selectionModel() || selectionModel()->selectedRows().size() > 1)
        {
            return;
        }

        if(!indexAt(event->pos()).isValid())
        {
            return;
        }

        QMenu customMenu;
        Platform::getInstance()->initMenu(&customMenu, "CustomMenu");
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

            if (access >= MegaShare::ACCESS_FULL && proxyModel->canBeDeleted() && node->isNodeKeyDecrypted())
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

#if QT_VERSION < QT_VERSION_CHECK(5, 15, 0)
void NodeSelectorTreeView::onCurrentRowChanged(const QModelIndex &current, const QModelIndex &previous)
{
    Q_UNUSED(previous)
        Qt::KeyboardModifiers modifiers = QGuiApplication::keyboardModifiers();
        if(modifiers & Qt::ControlModifier || modifiers & Qt::ShiftModifier || state() == QAbstractItemView::DragSelectingState)
        {
            return;
        }

        QItemSelectionModel::SelectionFlags flags = QItemSelectionModel::ClearAndSelect|QItemSelectionModel::Rows;
        selectionModel()->select(current, flags);
}
#endif


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

