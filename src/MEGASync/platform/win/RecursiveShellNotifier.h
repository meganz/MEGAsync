#ifndef RECURSIVESHELLNOTIFIER_H
#define RECURSIVESHELLNOTIFIER_H

#include "ShellNotifier.h"

#include <memory>
#include <string>
#include <QString>

/**
 * @brief Uses baseNotifier to notify for every folder
 * level recursively
 */
class RecursiveShellNotifier : public ShellNotifierDecorator
{
public:
    RecursiveShellNotifier(std::shared_ptr<AbstractShellNotifier> baseNotifier);
    virtual ~RecursiveShellNotifier() = default;

    void notify(const QString &path) override;

private:
    static void findFoldersRecursively(const QString& path, QStringList& folders);
};

#endif // RECURSIVESHELLNOTIFIER_H
