#include "StalledIssuesView.h"

#include <QHeaderView>
#include <QScrollBar>

StalledIssuesView::StalledIssuesView(QWidget *parent)
    :  LoadingSceneView<StalledIssueLoadingItem, QTreeView>(parent)
{
}
