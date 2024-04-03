#ifndef IQML_THEME_TARGET_H
#define IQML_THEME_TARGET_H

#include "Types.h"

#include <QString>

namespace  DTI
{
    class IQMLThemeTarget
    {
    public:
        virtual void deploy(const ThemedColourMap& themedColourMap) const = 0;
        virtual ~IQMLThemeTarget() = default;
    };
}

#endif
