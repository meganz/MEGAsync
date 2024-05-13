#ifndef DTI_TYPES_H
#define DTI_TYPES_H

#include <QMap>
#include <QString>

namespace DTI
{
    typedef QMap<QString, QString> CoreData;
    typedef QMap<QString, QString> ColorData;
    typedef QMap<QString, ColorData> ThemedColorData;
}

#endif
