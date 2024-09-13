#include "QmlManager.h"

#include "AccountInfoData.h"
#include "ApiEnums.h"
#include "AppStatsEvents.h"
#include "ChooseFile.h"
#include "ChooseFolder.h"
#include "LoginController.h"
#include "QmlClipboard.h"
#include "QmlDeviceName.h"
#include "QmlDialog.h"
#include "SyncInfo.h"
#include "QmlTheme.h"

#include <QDataStream>
#include <QQmlContext>
#include <QQueue>

static const QString DEFAULT_QML_INSTANCES_SUFFIX = QString::fromUtf8("Access");

QmlManager::QmlManager()
    : mEngine(new QQmlEngine())
{
    QObject::connect(mEngine, &QQmlEngine::warnings, [](const QList<QQmlError>& warnings) {
        for (const QQmlError& e : warnings) {
            QString message = QString::fromUtf8("QML error: ") + e.toString();
            ::mega::MegaApi::log(::mega::MegaApi::LOG_LEVEL_DEBUG, message.toUtf8().constData());
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
    delete mEngine;
    mEngine = nullptr;
}

void QmlManager::registerCommonQmlElements()
{
    mEngine->addImportPath(QString::fromUtf8("qrc:/"));

    qRegisterMetaTypeStreamOperators<QQueue<QString>>("QQueueQString");

    qmlRegisterUncreatableMetaObject(ApiEnums::staticMetaObject, "ApiEnums", 1, 0, "ApiEnums",
                                     QString::fromUtf8("Cannot create ApiEnums in QML"));

    qmlRegisterUncreatableType<LoginController>("LoginController", 1, 0, "LoginController",
                                                QString::fromUtf8("Cannot create WarningLevel in QML"));
    qmlRegisterUncreatableType<AppStatsEvents>("AppStatsEvents", 1, 0, "AppStatsEvents",
                                               QString::fromUtf8("Not creatable as it is an enum type"));
    qmlRegisterUncreatableType<SyncInfo>(
        "SyncInfo",
        1,
        0,
        "SyncInfo",
        QString::fromUtf8("Cannot register SyncInfo::SyncOrigin in QML"));
    qRegisterMetaType<AppStatsEvents::EventType>();

    qmlRegisterSingletonType<QmlClipboard>("QmlClipboard", 1, 0, "QmlClipboard", &QmlClipboard::qmlInstance);
    qmlRegisterSingletonType<AccountInfoData>("AccountInfoData", 1, 0, "AccountInfoData", AccountInfoData::instance);

    qmlRegisterType<QmlDialog>("QmlDialog", 1, 0, "QmlDialog");
    qmlRegisterType<QmlDeviceName>("QmlDeviceName", 1, 0, "QmlDeviceName");
    qmlRegisterType<ChooseLocalFolder>("ChooseLocalFolder", 1, 0, "ChooseLocalFolder");
    qmlRegisterType<ChooseLocalFile>("ChooseLocalFile", 1, 0, "ChooseLocalFile");

    setRootContextProperty(QString::fromUtf8("themeManager"), new QmlTheme(mEngine));
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
