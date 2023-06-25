#ifndef STALLEDISSUESVIEW_H
#define STALLEDISSUESVIEW_H

#include <ViewLoadingScene.h>
#include <StalledIssueLoadingItem.h>

#include <QTreeView>

class StalledIssuesView : public LoadingSceneView<StalledIssueLoadingItem, QTreeView>
{
    Q_OBJECT
public:
    StalledIssuesView(QWidget* parent);
};

#endif // STALLEDISSUESVIEW_H
