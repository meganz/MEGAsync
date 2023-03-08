#include "ShellNotifier.h"

#include "megaapi.h"

using namespace mega;

AbstractShellNotifier::AbstractShellNotifier()
    : QObject{nullptr}
{
}

void AbstractShellNotifier::logNotify(const char* tag, const QString &path)
{
    QString message = QString::fromUtf8("%1 : notifying %2").arg(QString::fromUtf8(tag), path);
    MegaApi::log(MegaApi::LOG_LEVEL_DEBUG, message.toUtf8().constData());
}

ShellNotifierDecorator::ShellNotifierDecorator(std::shared_ptr<AbstractShellNotifier> baseNotifier)
    : mBaseNotifier(baseNotifier)
{
}

void SignalShellNotifier::notify(const QString &path)
{
    logNotify("SignalShellNotifier", path);
    emit shellNotificationProcessed();
}
