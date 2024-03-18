#ifndef QMLMANAGER_H
#define QMLMANAGER_H

#include <memory>

#include <QQmlEngine>
#include <QObject>

class QmlManager : public QObject
{
    Q_OBJECT

public:
    static std::shared_ptr<QmlManager> instance();
    QmlManager(const QmlManager&) = delete;
    QmlManager& operator=(const QmlManager&) = delete;

    QQmlEngine* qmlEngine();
    void deleteEngine();

    void setRootContextProperty(QObject* value);
    void setRootContextProperty(const QString& name, QObject* value);
    void setRootContextProperty(const QString& name, const QVariant& value);

private:
    QQmlEngine* mEngine;

    QmlManager();
    void registerCommonQMLElements();
};

#endif // QMLMANAGER_H
