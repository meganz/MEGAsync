#ifndef DESIGN_TOKENS_TYPES_H
#define DESIGN_TOKENS_TYPES_H

#include <QMap>
#include <QString>

namespace DTI
{
    using ColorData = QMap<QString, QString>;
    using ThemedColorData = QMap<QString, ColorData>;

    struct DesignAssets
    {
        ThemedColorData colorTokens;
    };
}

#endif
