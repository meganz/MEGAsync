#ifndef QMLCLIPBOARD_H
#define QMLCLIPBOARD_H

#include <QObject>
#include <QQmlEngine>

class QmlClipboard : public QObject
{
    Q_OBJECT
    QML_ELEMENT
    QML_SINGLETON
    Q_DISABLE_COPY(QmlClipboard)

public:
    static QObject* qmlInstance(QQmlEngine* engine, QJSEngine* scriptEngine);

    Q_INVOKABLE void setText(const QString &from);
    Q_INVOKABLE QString text();

private:
    explicit QmlClipboard(QObject* parent = nullptr);
};

#endif // QMLCLIPBOARD_H


