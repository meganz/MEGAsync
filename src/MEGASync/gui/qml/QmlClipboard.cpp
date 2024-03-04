#include <QClipboard>
#include <QApplication>

#include "QmlClipboard.h"

QmlClipboard::QmlClipboard(QObject *parent)
    : QObject(parent) {}

QObject* QmlClipboard::qmlInstance(QQmlEngine* engine, QJSEngine* scriptEngine)
{
    Q_UNUSED(scriptEngine);

    QmlClipboard* instance = new QmlClipboard(engine);
    return instance;
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
