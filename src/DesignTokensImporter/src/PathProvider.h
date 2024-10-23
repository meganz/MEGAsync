#ifndef PATH_PROVIDER_H
#define PATH_PROVIDER_H

#include <QString>

namespace DTI
{
    namespace PathProvider
    {
        const QString RELATIVE_UI_PATH = QString::fromLatin1("/gui");
        const QString RELATIVE_MEGASYNC_PATH = QString::fromLatin1("../MEGASync");
        const QString RELATIVE_COLOR_DIR_PATH = RELATIVE_UI_PATH + QString::fromLatin1("/colors");
        const QString RELATIVE_DESIGN_TOKENS_FILE_PATH = QString::fromLatin1("../DesignTokensImporter/megadesignassets/tokens.json");
        const QString JSON_NAME_FILTER = QString::fromLatin1("*.json");
        const QString COLOR_THEMED_TOKENS_FILE_NAME = QString::fromLatin1("ColorThemedTokens.json");
    }
}

#endif
