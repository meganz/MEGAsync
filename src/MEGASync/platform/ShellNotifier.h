#ifndef SHELLNOTIFIER_H
#define SHELLNOTIFIER_H

#include <memory>
#include <QObject>

class AbstractShellNotifier : public QObject
{
    Q_OBJECT
public:
    AbstractShellNotifier();

    virtual void notify(const QString& path) = 0;

protected:
    static void logNotify(const char *tag, const QString& path);

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

    void notify(const QString& path) override;
};

#endif // SHELLNOTIFIER_H
