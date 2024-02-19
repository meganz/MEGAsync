#ifndef IQML_STYLE_TARGET_H
#define IQML_STYLE_TARGET_H

#include "StyleDefinitions.h"

#include <QString>

namespace  DTI
{
    class IQMLStyleTarget
    {
    public:
        virtual void deploy(QString theme, const ColourMap& colourMap) const = 0;
        virtual ~IQMLStyleTarget() = default;
    };
}

#endif
