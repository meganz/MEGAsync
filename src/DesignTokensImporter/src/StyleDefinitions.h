#ifndef STYLEDEFINITIONS_H
#define STYLEDEFINITIONS_H

#include <QString>


namespace DTI
{
typedef QMap<QString, QString> PropertiesMap;
typedef QMap<QString, PropertiesMap> ButtonStateStyleMap;
typedef QMap<QString, ButtonStateStyleMap> ImageThemeStyleInfo;
}

#endif // STYLEDEFINITIONS_H
