#include "ScreenshotComponent.h"

static bool qmlRegistrationDone = false;

ScreenshotComponent::ScreenshotComponent(QObject* parent):
    QMLComponent(parent),
    mScreenshotController(std::make_unique<ScreenshotAccessController>())
{
    registerQmlModules();

    QmlManager::instance()->setRootContextProperty(QString::fromLatin1("screenshotComponentAccess"),
                                                   this);
}

QUrl ScreenshotComponent::getQmlUrl()
{
    return QUrl(QString::fromUtf8("qrc:/screenshots/Screenshot.qml"));
}

void ScreenshotComponent::registerQmlModules()
{
    if (!qmlRegistrationDone)
    {
        qmlRegisterModule("ScreenshotComponent", 1, 0);

        qmlRegistrationDone = true;
    }
}
