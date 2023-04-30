#include "StalledIssuesView.h"

#include <QHeaderView>
#include <QScrollBar>

StalledIssuesView::StalledIssuesView(QWidget *parent)
    :  LoadingSceneView<StalledIssueLoadingItem, QTreeView>(parent)
{
    verticalScrollBar()->setSingleStep(1);
}

void StalledIssuesView::closeEditor(QWidget *editor, QAbstractItemDelegate::EndEditHint hint)
{
    QTreeView::closeEditor(editor, hint);
}
