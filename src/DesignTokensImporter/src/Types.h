#ifndef TYPES_H
#define TYPES_H

#include <QMap>
#include <QString>

namespace DTI
{
typedef QMap<QString, QString> PropertiesMap;
typedef QMap<QString, PropertiesMap> ButtonStateStyleMap;
typedef QMap<QString, ButtonStateStyleMap> ImageThemeStyleData;
typedef QMap<QString, QString> ColourMap;
typedef QMap<QString, ColourMap> FilePathColourMap;

struct ImageThemeStyleInfo
{
    QString key;
    QString cssSelector;
    ButtonStateStyleMap styleMap;
};

enum class Targets
{
    ColorStyle,
    ImageStyle
};
}

#endif // TYPES_H
