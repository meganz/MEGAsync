#include "PathProvider.h"
using namespace DTI;

//paths
const QString PathProvider::RELATIVE_GENERATED_PATH = QString::fromLatin1("../DesignTokensImporter/generated");
const QString PathProvider::RELATIVE_TOKENS_PATH = QString::fromLatin1("../DesignTokensImporter/tokens");
const QString PathProvider::RELATIVE_UI_PATH = QString::fromLatin1("/gui");
const QString PathProvider::RELATIVE_IMAGES_PATH = PathProvider::RELATIVE_UI_PATH + QString::fromLatin1("/images");
const QString PathProvider::RELATIVE_SVG_PATH = PathProvider::RELATIVE_IMAGES_PATH + QString::fromLatin1("/svg");
const QString PathProvider::RELATIVE_SVG_QRC_PATH = QString::fromLatin1("/gui/svg.qrc");
const QString PathProvider::RELATIVE_GENERATED_SVG_DIR_PATH =  QString::fromLatin1("/gui/images/svg/");
const QString PathProvider::RELATIVE_GUI_PRI_PATH = QString::fromLatin1("/gui/gui.pri");
const QString PathProvider::RELATIVE_RESOURCE_FILE_IMAGES_PATH = QString::fromLatin1(":/images/svg");
const QString PathProvider::RELATIVE_CMAKE_FILE_LIST_DIR_PATH =  QString::fromLatin1("../../contrib/cmake");


//filters
const QString PathProvider::JSON_NAME_FILTER =  QString::fromLatin1("*.json");
const QString PathProvider::UI_NAME_FILTER =  QString::fromLatin1("*.ui");
const QString PathProvider::SVG_NAME_FILTER =  QString::fromLatin1("*.svg");


//file extensions
const QString PathProvider::SVG_FILE_EXTENSION =  QString::fromLatin1(".svg");
