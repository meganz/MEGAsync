#include "QmlManager.h"

#include "ApiEnums.h"
#include "ColorTheme.h"
#include "QmlClipboard.h"

#include "LoginController.h"

#include <QQmlContext>
#include <QQueue>
#include <QDataStream>

static const QString DEFAULT_QML_INSTANCES_SUFFIX = QString::fromUtf8("Access");

QmlManager::QmlManager()
    : QObject()
    , mEngine(new QQmlEngine())
{
    registerCommonQMLElements();
}

std::shared_ptr<QmlManager> QmlManager::instance()
{
    static std::shared_ptr<QmlManager> manager(new QmlManager());
    return manager;
}

QQmlEngine* QmlManager::qmlEngine()
{
    return mEngine;
}

void QmlManager::deleteEngine()
{
    delete mEngine;
    mEngine = nullptr;
}

void QmlManager::registerCommonQMLElements()
{
    mEngine->addImportPath(QString::fromUtf8("qrc:/"));

    qRegisterMetaTypeStreamOperators<QQueue<QString>>("QQueueQString");
    qmlRegisterSingletonType<QmlClipboard>("QmlClipboard", 1, 0, "QmlClipboard", &QmlClipboard::qmlInstance);
    qmlRegisterUncreatableMetaObject(ApiEnums::staticMetaObject, "ApiEnums", 1, 0, "ApiEnums",
                                     QString::fromUtf8("Cannot create ApiEnums in QML"));
    qmlRegisterUncreatableType<LoginController>("LoginController", 1, 0, "LoginController",
                                                QString::fromUtf8("Cannot create WarningLevel in QML"));

    setRootContextProperty(QString::fromUtf8("colorStyle"), new ColorTheme(mEngine, this));
}

void QmlManager::setRootContextProperty(QObject* value)
{
    QString name(QString::fromUtf8(value->metaObject()->className()));
    if (name.isEmpty())
    {
        return;
    }

    // Example: "LoginController" -> "loginControllerAccess"
    name.replace(0, 1, name.at(0).toLower()).append(DEFAULT_QML_INSTANCES_SUFFIX);
    mEngine->rootContext()->setContextProperty(name, value);
}

void QmlManager::setRootContextProperty(const QString& name, QObject* value)
{
    mEngine->rootContext()->setContextProperty(name, value);
}

void QmlManager::setRootContextProperty(const QString& name, const QVariant& value)
{
    mEngine->rootContext()->setContextProperty(name, value);
}
