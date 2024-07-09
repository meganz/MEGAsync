#ifndef DESIGN_TARGET_H
#define DESIGN_TARGET_H

#include "Types.h"

namespace  DTI
{
    class IDesignTarget
    {
    public:
        virtual void deploy(const DesignAssets& designAssets) const = 0;
        virtual ~IDesignTarget() = default;
    };
}

#endif
