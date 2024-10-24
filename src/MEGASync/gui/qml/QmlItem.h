#ifndef QML_ITEM_H
#define QML_ITEM_H

#include <QQuickItem>

class QmlItem: public QQuickItem
{
    Q_OBJECT

    Q_PROPERTY(QVariantMap instances READ getInstances NOTIFY instancesChanged)

public:
    using QQuickItem::QQuickItem;
    virtual ~QmlItem() = default;

    void setInstance(QObject* instance);
    QVariantMap getInstances() const;

public slots:
    QObject* getInstance(const QString& name);

signals:
    void instancesChanged();

private:
    QVariantMap mInstances;

    void setInstance(const QString& name, QObject* instance);
};

#endif // QML_ITEM_H
