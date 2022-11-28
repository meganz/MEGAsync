#ifndef SHELLNOTIFIER_H
#define SHELLNOTIFIER_H

#include <memory>
#include <QObject>

class AbstractShellNotifier : public QObject
{
    Q_OBJECT
public:
    AbstractShellNotifier();

    virtual void notify(const std::string& path) = 0;

signals:
    void shellNotificationProcessed();
};

class ShellNotifierDecorator : public AbstractShellNotifier
{
public:
    ShellNotifierDecorator(std::shared_ptr<AbstractShellNotifier> baseNotifier);
    virtual ~ShellNotifierDecorator() = default;

protected:
    std::shared_ptr<AbstractShellNotifier> mBaseNotifier;
};

class SignalShellNotifier : public AbstractShellNotifier
{
public:
    SignalShellNotifier() = default;
    virtual ~SignalShellNotifier() = default;

    void notify(const std::string& path) override;
};

#endif // SHELLNOTIFIER_H
