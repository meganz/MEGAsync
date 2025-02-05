#ifndef STALLEDISSUESVIEW_H
#define STALLEDISSUESVIEW_H

#include "StalledIssueLoadingItem.h"
#include "ViewLoadingScene.h"

#include <QMouseEvent>
#include <QTimer>
#include <QTreeView>

class StalledIssuesView : public LoadingSceneView<StalledIssueLoadingItem, QTreeView>
{
    Q_OBJECT
public:
    StalledIssuesView(QWidget* parent);

signals:
    void scrollStopped();

protected slots:
    void onScrollMoved();

protected:
    void mousePressEvent(QMouseEvent *event) override;
    void keyPressEvent(QKeyEvent *event) override;

private:
    QTimer mScrollStop;
};

#endif // STALLEDISSUESVIEW_H
