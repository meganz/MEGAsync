#ifndef QMLCLIPBOARD_H
#define QMLCLIPBOARD_H

#include <QObject>
#include <QQmlEngine>

class QmlClipboard : public QObject
{
    Q_OBJECT
    Q_DISABLE_COPY(QmlClipboard)

public:
    static QmlClipboard* instance();

    static QObject* qmlInstance(QQmlEngine* engine, QJSEngine* scriptEngine);

    Q_INVOKABLE void setText(const QString &from);

    Q_INVOKABLE QString text();

private:
    static QmlClipboard* mThis;

    explicit QmlClipboard(QObject* parent = nullptr);

};

#endif // QMLCLIPBOARD_H


