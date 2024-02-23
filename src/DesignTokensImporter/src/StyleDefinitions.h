#ifndef STYLEDEFINITIONS_H
#define STYLEDEFINITIONS_H

#include <QString>


namespace DTI
{
typedef QMap<QString, QString> PropertiesMap;
typedef QMap<QString, PropertiesMap> ButtonStateStyleMap;
typedef QMap<QString, ButtonStateStyleMap> ImageThemeStyleInfo;
typedef QMap<QString, QString> ColourMap;
typedef QMap<QString, ColourMap> FilePathColourMap;
}

#endif // STYLEDEFINITIONS_H
