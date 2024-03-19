#ifndef QMLMANAGER_H
#define QMLMANAGER_H

#include <QQmlEngine>
#include <QObject>

#include <memory>

template <class Type>
class QmlDialogWrapper;

class QmlManager : public QObject
{
    Q_OBJECT

public:
    static std::shared_ptr<QmlManager> instance();

    QmlManager(const QmlManager&) = delete;
    QmlManager& operator=(const QmlManager&) = delete;

    void finish();

    void setRootContextProperty(QObject* value);
    void setRootContextProperty(const QString& name, QObject* value);
    void setRootContextProperty(const QString& name, const QVariant& value);

    void addImageProvider(const QString& id, QQmlImageProviderBase*);
    void removeImageProvider(const QString& id);

    void retranslate();

private:
    template <class Type>
    friend class QmlDialogWrapper;

    QQmlEngine* mEngine;

    QmlManager();
    void registerCommonQmlElements();

};

#endif // QMLMANAGER_H
