#ifndef QML_INSTANCES_MANAGER_H
#define QML_INSTANCES_MANAGER_H

#include <QObject>
#include <QVariant>

class QmlInstancesManager: public QObject
{
    Q_OBJECT

    Q_PROPERTY(QVariantMap instances READ getInstances NOTIFY instancesChanged)

public:
    virtual ~QmlInstancesManager() = default;

    void setInstance(QObject* instance);
    QVariantMap getInstances() const;

    template<class Type>
    void initInstances(QPointer<Type> wrapper)
    {
        setInstance(wrapper);
        auto instances = wrapper->getInstancesFromContext();
        foreach(auto instance, instances)
        {
            setInstance(instance);
        }
    }

signals:
    void instancesChanged();

private:
    QVariantMap mInstances;

    void setInstance(const QString& name, QObject* instance);
};

#endif // QML_INSTANCES_MANAGER_H
