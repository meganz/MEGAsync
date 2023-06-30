#include "ShellNotifier.h"

AbstractShellNotifier::AbstractShellNotifier()
    : QObject{nullptr}
{
}

ShellNotifierDecorator::ShellNotifierDecorator(std::shared_ptr<AbstractShellNotifier> baseNotifier)
    : mBaseNotifier(baseNotifier)
{
}

void SignalShellNotifier::notify(const QString &path)
{
    emit shellNotificationProcessed();
}
