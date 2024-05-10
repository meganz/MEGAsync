#ifndef PATHPROVIDER_H
#define PATHPROVIDER_H

#include <QString>

namespace DTI
{
    class PathProvider
    {
    public:
        PathProvider() = delete;

        static const QString RELATIVE_MEGASYNC_PATH;
        static const QString RELATIVE_GENERATED_PATH;
        static const QString RELATIVE_DESIGN_TOKENS_FILE_PATH;
        static const QString RELATIVE_UI_PATH;
        static const QString RELATIVE_UI_WIN_PATH;
        static const QString RELATIVE_UI_LINUX_PATH;
        static const QString RELATIVE_UI_MAC_PATH;
        static const QString RELATIVE_QRC_MAC_PATH;
        static const QString RELATIVE_QRC_WINDOWS_PATH;
        static const QString RELATIVE_QRC_LINUX_PATH;
        static const QString RELATIVE_THEMES_DIR_PATH;
        static const QString RELATIVE_CSS_WIN_PATH;
        static const QString RELATIVE_CSS_LINUX_PATH;
        static const QString RELATIVE_CSS_MAC_PATH;
        static const QString RELATIVE_HASHES_PATH;

        static const QString JSON_NAME_FILTER;
        static const QString UI_NAME_FILTER;
        static const QString CSS_NAME_FILTER;

        static const QString CSS_FILE_EXTENSION;
        static const QString UI_FILE_EXTENSION;
    };
}

#endif // PATHPROVIDER_H
