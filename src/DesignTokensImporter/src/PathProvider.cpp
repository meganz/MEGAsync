#include "PathProvider.h"

using namespace DTI;

//paths
const QString PathProvider::RELATIVE_GENERATED_PATH = QString::fromLatin1("../DesignTokensImporter/generated");
const QString PathProvider::RELATIVE_CORE_FILE_PATH = QString::fromLatin1("../DesignTokensImporter/megadesignassets/core.json");
const QString PathProvider::RELATIVE_COLOR_TOKENS_PATH = QString::fromLatin1("../DesignTokensImporter/megadesignassets/colors");
const QString PathProvider::RELATIVE_UI_PATH = QString::fromLatin1("/gui");
const QString PathProvider::RELATIVE_IMAGES_PATH = PathProvider::RELATIVE_UI_PATH + QString::fromLatin1("/images");
const QString PathProvider::RELATIVE_SVG_PATH = PathProvider::RELATIVE_IMAGES_PATH + QString::fromLatin1("/svg");
const QString PathProvider::RELATIVE_SVG_QRC_PATH = QString::fromLatin1("/gui/svg.qrc");
const QString PathProvider::RELATIVE_GENERATED_SVG_DIR_PATH =  QString::fromLatin1("/gui/images/svg/");
const QString PathProvider::RELATIVE_GUI_PRI_PATH = QString::fromLatin1("/gui/gui.pri");
const QString PathProvider::RELATIVE_RESOURCE_FILE_IMAGES_PATH = QString::fromLatin1(":/images/svg");
const QString PathProvider::RELATIVE_UI_WIN_PATH = RELATIVE_UI_PATH + QString::fromLatin1("/win");
const QString PathProvider::RELATIVE_UI_LINUX_PATH =  RELATIVE_UI_PATH + QString::fromLatin1("/linux");
const QString PathProvider::RELATIVE_UI_MAC_PATH = RELATIVE_UI_PATH + QString::fromLatin1("/macx");
const QString PathProvider::RELATIVE_QRC_MAC_PATH = RELATIVE_UI_PATH + QString::fromLatin1("/Resources_macx.qrc");
const QString PathProvider::RELATIVE_QRC_WINDOWS_PATH =  RELATIVE_UI_PATH + QString::fromLatin1("/Resources_win.qrc");
const QString PathProvider::RELATIVE_QRC_LINUX_PATH = RELATIVE_UI_PATH +  QString::fromLatin1("/Resources_linux.qrc");
const QString PathProvider::RELATIVE_THEMES_DIR_PATH = RELATIVE_UI_PATH + QString::fromLatin1("/themes");
const QString PathProvider::RELATIVE_STYLES_DIR_PATH = RELATIVE_UI_PATH + QString::fromLatin1("/themes/styles");
const QString PathProvider::RELATIVE_CSS_WIN_PATH = RELATIVE_STYLES_DIR_PATH + QString::fromLatin1("/win");
const QString PathProvider::RELATIVE_CSS_LINUX_PATH = RELATIVE_STYLES_DIR_PATH + QString::fromLatin1("/linux");
const QString PathProvider::RELATIVE_CSS_MAC_PATH = RELATIVE_STYLES_DIR_PATH + QString::fromLatin1("/macx");
const QString PathProvider::RELATIVE_HASHES_PATH = RELATIVE_GENERATED_PATH + QString::fromLatin1("/hashes.json");
const QString PathProvider::RELATIVE_CMAKE_FILE_LIST_DIR_PATH =  QString::fromLatin1("../../contrib/cmake");


//filters
const QString PathProvider::JSON_NAME_FILTER =  QString::fromLatin1("*.json");
const QString PathProvider::UI_NAME_FILTER =  QString::fromLatin1("*.ui");
const QString PathProvider::SVG_NAME_FILTER =  QString::fromLatin1("*.svg");
const QString PathProvider::CSS_NAME_FILTER =  QString::fromLatin1("*.css");


//file extensions
const QString PathProvider::SVG_FILE_EXTENSION =  QString::fromLatin1(".svg");
const QString PathProvider::CSS_FILE_EXTENSION =  QString::fromLatin1(".css");
const QString PathProvider::UI_FILE_EXTENSION =  QString::fromLatin1(".ui");
