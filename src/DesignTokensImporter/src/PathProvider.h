#ifndef PATH_PROVIDER_H
#define PATH_PROVIDER_H

#include <QString>

namespace DTI
{
class PathProvider
{
public:
    static QString getUIPath();
    static QString getMegaSyncPath();
    static QString getColorDirPath();
    static QString getDesignTokensFilePath();
    static QString getJsonNameFilter();
    static QString getColorThemedTokensFileName();

    static void setUIPath(const QString& path);
    static void setMegaSyncPath(const QString& path);
    static void setDesignTokensFilePath(const QString& path);

private:
    static inline QString mUIPath;
    static inline QString mMegaSyncPath;
    static inline QString mDesignTokensFilePath;
    // Constants
    static const QString JSON_NAME_FILTER;
    static const QString COLOR_THEMED_TOKENS_FILE_NAME;
};
}

#endif
