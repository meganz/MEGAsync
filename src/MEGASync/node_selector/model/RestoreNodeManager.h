#ifndef RESTORENODEMANAGER_H
#define RESTORENODEMANAGER_H

#include "megaapi.h"

#include <QObject>
#include <QPointer>

class NodeSelectorModel;

class RestoreNodeManager: public QObject
{
    Q_OBJECT

public:
    RestoreNodeManager(NodeSelectorModel* model, QObject* parent);
    ~RestoreNodeManager() = default;

public slots:
    void onRestoreClicked(const QList<mega::MegaHandle>& handles);

signals:
    void itemsRestoreRequested(const QList<mega::MegaHandle>& handles);

private:
    QList<mega::MegaHandle> mRestoredItems;
    QPointer<NodeSelectorModel> mModel;
};
#endif // RESTORENODEMANAGER_H
