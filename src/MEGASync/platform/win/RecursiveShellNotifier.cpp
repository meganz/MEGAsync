#include "RecursiveShellNotifier.h"

#include <QDir>

RecursiveShellNotifier::RecursiveShellNotifier(std::shared_ptr<AbstractShellNotifier> baseNotifier)
    : ShellNotifierDecorator(baseNotifier)
{
}

void RecursiveShellNotifier::notify(const std::string &path)
{
    mBaseNotifier->notify(path);

    QStringList foldersToNotify;
    findFoldersRecursively(toQstring(path), foldersToNotify);

    for (const auto& folder : qAsConst(foldersToNotify))
    {
        mBaseNotifier->notify(toStdString(folder));
    }

    emit shellNotificationProcessed();
}

void RecursiveShellNotifier::findFoldersRecursively(const QString &path, QStringList &folders)
{
    QDir dir(path);
    QFileInfoList children = dir.entryInfoList(QDir::AllDirs | QDir::NoDotAndDotDot);
    for (const auto& child : qAsConst(children))
    {
        QString childPath = QDir::toNativeSeparators(child.absoluteFilePath());
        folders.push_back(childPath);
        findFoldersRecursively(childPath, folders);
    }
}

QString RecursiveShellNotifier::toQstring(const std::string& value)
{
    return QString::fromUtf16(reinterpret_cast<const ushort*>(value.data()), static_cast<int>(value.size() / 2));
}

std::string RecursiveShellNotifier::toStdString(const QString& value)
{
    std::string stdValue;
    stdValue.assign(reinterpret_cast<const char*>(value.utf16()), value.size() * sizeof(wchar_t));
    stdValue += '\000';
    return stdValue;
}
