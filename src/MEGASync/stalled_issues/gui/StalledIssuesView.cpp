#include "StalledIssuesView.h"

#include <QHeaderView>

StalledIssuesView::StalledIssuesView(QWidget *parent)
    :  QTreeView(parent)
{
}

void StalledIssuesView::closeEditor(QWidget *editor, QAbstractItemDelegate::EndEditHint hint)
{
    QTreeView::closeEditor(editor, hint);
}
