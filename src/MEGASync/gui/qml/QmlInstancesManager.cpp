#include "QmlInstancesManager.h"

namespace
{
const QLatin1String DEFAULT_QML_INSTANCES_SUFFIX("Access");
}

void QmlInstancesManager::setInstance(const QString& name, QObject* instance)
{
    if (mInstances[name] != QVariant::fromValue(instance))
    {
        mInstances[name] = QVariant::fromValue(instance);
        emit instancesChanged();
    }
}

void QmlInstancesManager::setInstance(QObject* instance)
{
    QString name(QString::fromUtf8(instance->metaObject()->className()));
    if (name.isEmpty())
    {
        return;
    }

    // Example: "Surveys" -> "surveysAccess"
    name.replace(0, 1, name.at(0).toLower()).append(DEFAULT_QML_INSTANCES_SUFFIX);
    setInstance(name, instance);
}

QVariantMap QmlInstancesManager::getInstances() const
{
    return mInstances;
}
