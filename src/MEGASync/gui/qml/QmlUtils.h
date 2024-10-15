#ifndef QMLUTILS_H
#define QMLUTILS_H

#include <QObject>
#include <QQmlEngine>

class QmlUtils: public QObject
{
    Q_OBJECT
    QML_ELEMENT
    QML_SINGLETON
    Q_DISABLE_COPY(QmlUtils)

public:
    static QmlUtils* getQmlInstance(QQmlEngine* engine, QJSEngine* scriptEngine);

    Q_INVOKABLE QString getCurrentDeviceID();

private:
    explicit QmlUtils(QObject* parent = nullptr);
};

#endif // QMLUTILS_H
