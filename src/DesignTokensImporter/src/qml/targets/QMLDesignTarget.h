#ifndef IQML_DESIGN_TARGET_H
#define IQML_DESIGN_TARGET_H

#include "Types.h"

#include <QString>

namespace  DTI
{
    class IQMLDesignTarget
    {
    public:
        virtual void deploy(const DesignAssets& designAssets) const = 0;
        virtual ~IQMLDesignTarget() = default;
    };
}

#endif
