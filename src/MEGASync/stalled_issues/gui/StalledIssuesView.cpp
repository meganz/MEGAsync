#include "StalledIssuesView.h"

#include <QHeaderView>
#include <QScrollBar>

StalledIssuesView::StalledIssuesView(QWidget *parent)
    :  LoadingSceneView<StalledIssueLoadingItem, QTreeView>(parent)
{
}

void StalledIssuesView::mousePressEvent(QMouseEvent *event)
{
    QPoint pos = event->pos();
    QPersistentModelIndex index = indexAt(pos);

    if(!index.parent().isValid())
    {
        QItemSelectionModel::SelectionFlags command = selectionCommand(index, event);
        selectionModel()->select(index, command);
    }
    else
    {
        QItemSelectionModel::SelectionFlags command = selectionCommand(index.parent(), event);
        selectionModel()->select(index.parent(), command);
    }
}
