#ifndef DTI_PATH_PROVIDER_H
#define DTI_PATH_PROVIDER_H

#include <QString>

namespace DTI
{
    class PathProvider
    {
    public:
        PathProvider() = delete;

        static const QString RELATIVE_UI_PATH;
        static const QString RELATIVE_MEGASYNC_PATH;
        static const QString RELATIVE_THEMES_DIR_PATH;
        static const QString RELATIVE_DESIGN_TOKENS_FILE_PATH;
        static const QString JSON_NAME_FILTER;
        static const QString COLOR_THEMED_TOKENS_FILE_NAME;
    };
}

#endif // PATHPROVIDER_H
