#ifndef STALLEDISSUESVIEW_H
#define STALLEDISSUESVIEW_H

#include <ViewLoadingScene.h>
#include <StalledIssueLoadingItem.h>

#include <QTreeView>
#include <QMouseEvent>
#include <QTimer>

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
