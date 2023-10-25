#include <QClipboard>
#include <QApplication>

#include "QmlClipboard.h"

QmlClipboard::QmlClipboard(QObject *parent)
    : QObject(parent) {}

QmlClipboard* QmlClipboard::instance()
{
    static QmlClipboard instance;
    return &instance;
}

QObject* QmlClipboard::qmlInstance(QQmlEngine* engine, QJSEngine* scriptEngine)
{
    Q_UNUSED(engine);
    Q_UNUSED(scriptEngine);

    return QmlClipboard::instance();
}

void QmlClipboard::setText(const QString& from)
{
    QClipboard* clipboard = QApplication::clipboard();
    clipboard->setText(from);
}

QString QmlClipboard::text()
{
    QClipboard* clipboard = QApplication::clipboard();
    return clipboard->text();
}
