#include "ChooseFile.h"

#include "Platform.h"

#include "megaapi.h"

#include <QDir>

ChooseLocalFile::ChooseLocalFile(QObject *parent)
    : QObject(parent)
    , mTitle(QString::fromUtf8(""))
{
}

void ChooseLocalFile::openFileSelector(const QString& folderPath)
{
    auto openFromFolder = QDir::toNativeSeparators(Utilities::getDefaultBasePath());

    if (!folderPath.isEmpty())
    {
        openFromFolder = QDir::toNativeSeparators(folderPath);
        QDir openFromFolderDir(openFromFolder);
        if (openFromFolderDir.cdUp())
        {
            openFromFolder = openFromFolderDir.path();
        }
        else
        {
            openFromFolder = QDir::toNativeSeparators(Utilities::getDefaultBasePath());
        }
    }

    SelectorInfo info;
    info.title = mTitle;
    info.defaultDir = openFromFolder;
    info.canCreateDirectories = true;

    QPointer<const QObject> context = this;
    info.func = [this, context, openFromFolder](QStringList selection)
    {
        if(context && !selection.isEmpty())
        {
            QString fPath = selection.first();
            auto folder = QDir::toNativeSeparators(QDir(fPath).canonicalPath());
            if(!folder.isNull() && !folder.isEmpty())
            {
                emit fileChoosen(folder);
            }
        }
    };

    Platform::getInstance()->fileSelector(info);
}

void ChooseLocalFile::openRelativeFileSelector(const QString& folderPath)
{
    auto openFromFolder = QDir::toNativeSeparators(Utilities::getDefaultBasePath());
    if (!folderPath.isEmpty())
    {
        openFromFolder = QDir::toNativeSeparators(folderPath);
    }

    SelectorInfo info;
    info.title = mTitle;
    info.multiSelection = false;
    info.defaultDir = openFromFolder;

    QPointer<const QObject> context = this;
    info.func = [this, context, openFromFolder](QStringList selection)
    {
        if(context && !selection.isEmpty())
        {
            QString fPath = selection.first();
            auto folder = QDir::toNativeSeparators(QDir(fPath).canonicalPath());
            if(!folder.isNull() && !folder.isEmpty())
            {
                emit fileChoosen(QDir(openFromFolder).relativeFilePath(folder));
            }
        }
    };

    Platform::getInstance()->fileSelector(info);
}
