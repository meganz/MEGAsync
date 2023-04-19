#include "ShellNotifier.h"

#include "megaapi.h"

using namespace mega;

AbstractShellNotifier::AbstractShellNotifier()
    : QObject{nullptr}
{
}

void AbstractShellNotifier::logNotify(const char* tag, const QString &path)
{
    // Don't actually log here under normal circumstances.
    // Imagine a million-node sync, that'll kill performance and make the logs unusable
    // Uncomment this for dev purposes only when needed for debugging

    //QString message = QString::fromUtf8("%1 : notifying %2").arg(QString::fromUtf8(tag), path);
    //MegaApi::log(MegaApi::LOG_LEVEL_DEBUG, message.toUtf8().constData());
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
