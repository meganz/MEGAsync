#ifndef STALLEDISSUESVIEW_H
#define STALLEDISSUESVIEW_H

#include <ViewLoadingScene.h>
#include <StalledIssueLoadingItem.h>

#include <QTreeView>
#include <QMouseEvent>

class StalledIssuesView : public LoadingSceneView<StalledIssueLoadingItem, QTreeView>
{
    Q_OBJECT
public:
    StalledIssuesView(QWidget* parent);

protected:
    void mousePressEvent(QMouseEvent *event) override;
};

#endif // STALLEDISSUESVIEW_H
