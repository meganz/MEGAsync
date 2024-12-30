#include "QmlItem.h"

QmlItem::QmlItem(QQuickItem* parent):
    QQuickItem(parent),
    mInstancesManager(new QmlInstancesManager())
{
    connect(mInstancesManager,
            &QmlInstancesManager::instancesChanged,
            this,
            &QmlItem::instancesManagerChanged);
}

QmlInstancesManager* QmlItem::getInstancesManager()
{
    return mInstancesManager;
}
