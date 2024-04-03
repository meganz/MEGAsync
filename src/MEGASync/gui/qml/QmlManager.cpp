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
    : mEngine(new QQmlEngine())
{
    QObject::connect(mEngine, &QQmlEngine::warnings, [](const QList<QQmlError>& warnings) {
        for (const QQmlError& e : warnings) {
            qDebug() << "Qml error: " << e.toString();
        }
    });

    registerCommonQmlElements();
}

std::shared_ptr<QmlManager> QmlManager::instance()
{
    static std::shared_ptr<QmlManager> manager(new QmlManager());
    return manager;
}

void QmlManager::finish()
{
    qDeleteAll(mEngine->children());
    delete mEngine;
    mEngine = nullptr;
}

void QmlManager::registerCommonQmlElements()
{
    mEngine->addImportPath(QString::fromUtf8("qrc:/"));

    qRegisterMetaTypeStreamOperators<QQueue<QString>>("QQueueQString");
    qmlRegisterSingletonType<QmlClipboard>("QmlClipboard", 1, 0, "QmlClipboard", &QmlClipboard::qmlInstance);
    qmlRegisterUncreatableMetaObject(ApiEnums::staticMetaObject, "ApiEnums", 1, 0, "ApiEnums",
                                     QString::fromUtf8("Cannot create ApiEnums in QML"));
    qmlRegisterUncreatableType<LoginController>("LoginController", 1, 0, "LoginController",
                                                QString::fromUtf8("Cannot create WarningLevel in QML"));

    setRootContextProperty(QString::fromUtf8("colorStyle"), new ColorTheme(mEngine, mEngine));
}

void QmlManager::setRootContextProperty(QObject* value)
{
    QString name(QString::fromUtf8(value->metaObject()->className()));
    if (name.isEmpty() || !mEngine)
    {
        return;
    }

    // Example: "LoginController" -> "loginControllerAccess"
    name.replace(0, 1, name.at(0).toLower()).append(DEFAULT_QML_INSTANCES_SUFFIX);
    mEngine->rootContext()->setContextProperty(name, value);
}

void QmlManager::setRootContextProperty(const QString& name, QObject* value)
{
    if(mEngine)
    {
        mEngine->rootContext()->setContextProperty(name, value);
    }
}

void QmlManager::setRootContextProperty(const QString& name, const QVariant& value)
{
    if(mEngine)
    {
        mEngine->rootContext()->setContextProperty(name, value);
    }
}

void QmlManager::addImageProvider(const QString& id, QQmlImageProviderBase* provider)
{
    if(mEngine)
    {
        mEngine->addImageProvider(id, provider);
    }
}

void QmlManager::removeImageProvider(const QString& id)
{
    if(mEngine)
    {
        mEngine->removeImageProvider(id);
    }
}

void QmlManager::retranslate()
{
    if(mEngine)
    {
        mEngine->retranslate();
    }
}

QQmlEngine* QmlManager::getEngine()
{
    return mEngine;
}
