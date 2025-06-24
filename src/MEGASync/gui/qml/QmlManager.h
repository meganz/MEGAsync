#ifndef QML_MANAGER_H
#define QML_MANAGER_H

#include <QQmlEngine>

#include <memory>

class QmlManager
{
public:
    static std::shared_ptr<QmlManager> instance();

    QmlManager(const QmlManager&) = delete;
    QmlManager& operator=(const QmlManager&) = delete;

    void finish();

    void setRootContextProperty(QObject* value);
    void setRootContextProperty(const QString& name, QObject* value);
    void setRootContextProperty(const QString& name, const QVariant& value);

    bool isRootContextPropertySet(QObject* value);

    void addImageProvider(const QString& id, QQmlImageProviderBase*);
    void removeImageProvider(const QString& id);

    void retranslate();

    QQmlEngine* getEngine();

private:
    QQmlEngine* mEngine;

    QmlManager();
    void registerCommonQmlElements();
    QString getObjectRootContextName(QObject* value);
    void createPlatformSelectorsFlags();
};

#endif // QML_MANAGER_H
