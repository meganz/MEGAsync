#include "PathProvider.h"

#include <QDir>

namespace DTI
{
const QString PathProvider::JSON_NAME_FILTER = QString::fromLatin1("*.json");
const QString PathProvider::COLOR_THEMED_TOKENS_FILE_NAME =
    QString::fromLatin1("ColorThemedTokens.json");

QString PathProvider::getUIPath()
{
    return mUIPath;
}

QString PathProvider::getMegaSyncPath()
{
    return mMegaSyncPath;
}

QString PathProvider::getColorDirPath()
{
    return mUIPath + QString::fromLatin1("/colors");
}

QString PathProvider::getDesignTokensFilePath()
{
    return mDesignTokensFilePath;
}

QString PathProvider::getJsonNameFilter()
{
    return JSON_NAME_FILTER;
}

QString PathProvider::getColorThemedTokensFileName()
{
    return COLOR_THEMED_TOKENS_FILE_NAME;
}

void PathProvider::setUIPath(const QString& path)
{
    mUIPath = path;
}

void PathProvider::setMegaSyncPath(const QString& path)
{
    mMegaSyncPath = path;
}

void PathProvider::setDesignTokensFilePath(const QString& path)
{
    mDesignTokensFilePath = path;
}
}
