#ifndef PATHPROVIDER_H
#define PATHPROVIDER_H

#include <QString>
namespace DTI
{
class PathProvider
{
public:
    PathProvider() = delete;

    //paths
    static const QString RELATIVE_GENERATED_PATH;
    static const QString RELATIVE_TOKENS_PATH;
    static const QString RELATIVE_UI_PATH;
    static const QString RELATIVE_IMAGES_PATH;
    static const QString RELATIVE_SVG_PATH;
    static const QString RELATIVE_SVG_QRC_PATH;
    static const QString RELATIVE_GENERATED_SVG_DIR_PATH;
    static const QString RELATIVE_GUI_PRI_PATH;
    static const QString RELATIVE_RESOURCE_FILE_IMAGES_PATH;
    static const QString RELATIVE_CMAKE_FILE_LIST_DIR_PATH;

    //filters
    static const QString JSON_NAME_FILTER;
    static const QString UI_NAME_FILTER;
    static const QString SVG_NAME_FILTER;

    //file extensions
    static const QString SVG_FILE_EXTENSION;
};
}

#endif // PATHPROVIDER_H
