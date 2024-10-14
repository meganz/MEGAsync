#include "QmlUtils.h"

#include "MegaApplication.h"
#include "Utilities.h"

QmlUtils::QmlUtils(QObject* parent):
    QObject(parent)
{}

QmlUtils* QmlUtils::getQmlInstance(QQmlEngine* qmlEngine, QJSEngine* jsEngine)
{
    Q_UNUSED(jsEngine);
    static auto instance = new QmlUtils(qmlEngine);
    return instance;
}

QString QmlUtils::getCurrentDeviceID()
{
    if (auto megaAPI = MegaSyncApp->getMegaApi())
    {
        std::unique_ptr<const char> rawDeviceId{megaAPI->getDeviceId()};
        return QString::fromLatin1(rawDeviceId.get());
    }
    return QString{};
}
