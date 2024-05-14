#include "PathProvider.h"

using namespace DTI;

const QString PathProvider::RELATIVE_UI_PATH = QString::fromLatin1("/gui");
const QString PathProvider::RELATIVE_MEGASYNC_PATH = QString::fromLatin1("../MEGASync");
const QString PathProvider::RELATIVE_COLOR_DIR_PATH = RELATIVE_UI_PATH + QString::fromLatin1("/colors");
const QString PathProvider::RELATIVE_DESIGN_TOKENS_FILE_PATH = QString::fromLatin1("../DesignTokensImporter/megadesignassets/tokens.json");
const QString PathProvider::JSON_NAME_FILTER =  QString::fromLatin1("*.json");
const QString PathProvider::COLOR_THEMED_TOKENS_FILE_NAME = QString::fromLatin1("ColorThemedTokens.json");
