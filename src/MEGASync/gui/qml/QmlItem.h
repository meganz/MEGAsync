#ifndef QML_ITEM_H
#define QML_ITEM_H

#include "QmlInstancesManager.h"

#include <QQuickItem>

class QmlItem: public QQuickItem
{
    Q_OBJECT

    Q_PROPERTY(QmlInstancesManager* instancesManager READ getInstancesManager NOTIFY
                   instancesManagerChanged)

public:
    explicit QmlItem(QQuickItem* parent = nullptr);
    virtual ~QmlItem() = default;

public slots:
    QmlInstancesManager* getInstancesManager();

signals:
    void instancesManagerChanged();

private:
    QPointer<QmlInstancesManager> mInstancesManager;
};

#endif // QML_ITEM_H
