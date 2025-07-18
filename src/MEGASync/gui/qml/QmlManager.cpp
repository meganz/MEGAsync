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
#include "QmlDialogManager.h"
#include "QmlItem.h"
#include "QmlTheme.h"
#include "QmlUtils.h"
#include "SyncInfo.h"

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
    qRegisterMetaType<SyncInfo::SyncOrigin>();
    qRegisterMetaType<AppStatsEvents::EventType>();

    qmlRegisterSingletonType<QmlUtils>("QmlUtils", 1, 0, "QmlUtils", &QmlUtils::getQmlInstance);
    qmlRegisterSingletonType<QmlClipboard>("QmlClipboard", 1, 0, "QmlClipboard", &QmlClipboard::qmlInstance);
    qmlRegisterSingletonType<AccountInfoData>("AccountInfoData", 1, 0, "AccountInfoData", AccountInfoData::instance);
    qmlRegisterSingletonType<QmlDialogManager>("QmlDialogManager",
                                               1,
                                               0,
                                               "QmlDialogManager",
                                               QmlDialogManager::getQmlInstance);

    qmlRegisterType<QmlDialog>("QmlDialog", 1, 0, "QmlDialog");
    qmlRegisterType<QmlItem>("QmlItem", 1, 0, "QmlItem");
    qmlRegisterType<QmlDeviceName>("QmlDeviceName", 1, 0, "QmlDeviceName");
    qmlRegisterType<ChooseLocalFolder>("ChooseLocalFolder", 1, 0, "ChooseLocalFolder");
    qmlRegisterType<ChooseRemoteFolder>("ChooseRemoteFolder", 1, 0, "ChooseRemoteFolder");
    qmlRegisterType<ChooseLocalFile>("ChooseLocalFile", 1, 0, "ChooseLocalFile");

    setRootContextProperty(QString::fromUtf8("themeManager"), new QmlTheme(mEngine));

    createPlatformSelectorsFlags();
}

QString QmlManager::getObjectRootContextName(QObject* value)
{
    QString name(QString::fromUtf8(value->metaObject()->className()));
    if (name.isEmpty() || !mEngine)
    {
        return QString();
    }

    if (!name.isEmpty())
    {
        name.replace(0, 1, name.at(0).toLower());
    }

    // Example: "LoginController" -> "loginControllerAccess"
    name.append(DEFAULT_QML_INSTANCES_SUFFIX);

    return name;
}

void QmlManager::createPlatformSelectorsFlags()
{
    QString platform;

    bool isWindows = false;
#ifdef Q_OS_WINDOWS
    isWindows = true;
    platform = QString::fromLatin1("windows");
#endif
    setRootContextProperty(QString::fromUtf8("isWindows"), isWindows);

    bool isLinux = false;
#ifdef Q_OS_LINUX
    platform = QString::fromLatin1("linux");
    isLinux = true;
#endif
    setRootContextProperty(QString::fromUtf8("isLinux"), isLinux);

    bool isMACos = false;
#ifdef Q_OS_MACOS
    platform = QString::fromLatin1("macos");
    isMACos = true;
#endif
    setRootContextProperty(QString::fromUtf8("isMACos"), isMACos);

    setRootContextProperty(QString::fromUtf8("platform"), platform);
}

void QmlManager::setRootContextProperty(QObject* value)
{
    mEngine->rootContext()->setContextProperty(getObjectRootContextName(value), value);
}

bool QmlManager::isRootContextPropertySet(QObject* value)
{
    return mEngine->rootContext()->contextProperty(getObjectRootContextName(value)).isValid();
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
