#include "RecursiveShellNotifier.h"

#include <QDir>

extern bool WindowsPlatform_exiting;

RecursiveShellNotifier::RecursiveShellNotifier(std::shared_ptr<AbstractShellNotifier> baseNotifier)
    : ShellNotifierDecorator(baseNotifier)
{
}

void RecursiveShellNotifier::notify(const QString& path)
{
    mBaseNotifier->notify(path);

    QStringList foldersToNotify;
    findFoldersRecursively(path, foldersToNotify);

    for (const auto& folder: std::as_const(foldersToNotify))
    {
        mBaseNotifier->notify(folder);
        if (WindowsPlatform_exiting) return;
    }

    emit shellNotificationProcessed();
}

void RecursiveShellNotifier::findFoldersRecursively(const QString& path,
                                                    QStringList& folders,
                                                    int depth)
{
    // Junctions/symlinks can form cycles (e.g. a junction pointing to an ancestor),
    // which would make this recursion endless; the depth cap is a second line of defense.
    constexpr int MAX_RECURSION_DEPTH = 64;
    if (depth >= MAX_RECURSION_DEPTH)
    {
        return;
    }

    QDir dir(path);
    QFileInfoList children = dir.entryInfoList(QDir::AllDirs | QDir::NoDotAndDotDot);
    for (const auto& child: std::as_const(children))
    {
        if (WindowsPlatform_exiting) return;
        QString childPath = QDir::toNativeSeparators(child.absoluteFilePath());
        folders.push_back(childPath);
        if (child.isJunction() || child.isSymbolicLink())
        {
            continue;
        }
        findFoldersRecursively(childPath, folders, depth + 1);
    }
}
