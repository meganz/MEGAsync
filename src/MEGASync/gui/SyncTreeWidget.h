#ifndef SYNCTREEWIDGET_H
#define SYNCTREEWIDGET_H

#include <QObject>
#include <QTreeView>

class SyncTreeWidget : public QTreeView
{
    Q_OBJECT
public:
    SyncTreeWidget(QWidget *parent);
};

#endif // SYNCTREEWIDGET_H
