#ifndef DTI_TYPES_H
#define DTI_TYPES_H

#include <QMap>
#include <QString>

namespace DTI
{
    using CoreData = QMap<QString, QString>;
    using ColorData = QMap<QString, QString>;
    using ThemedColorData = QMap<QString, ColorData>;

    struct DesignAssets
    {
        ThemedColorData colorTokens;
    };
}

#endif
