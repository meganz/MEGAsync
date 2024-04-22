#ifndef TYPES_H
#define TYPES_H

#include <QMap>
#include <QString>

namespace DTI
{
typedef QMap<QString, QString> PropertiesMap;
typedef QMap<QString, QString> ColourMap;
typedef QMap<QString, QString> CoreMap;
typedef QMap<QString, ColourMap> ThemedColourMap;

enum class Targets
{
    ColorStyle
};
}

#endif // TYPES_H
