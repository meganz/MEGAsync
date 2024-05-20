#ifndef I_DESIGN_GENERATOR_H
#define I_DESIGN_GENERATOR_H

#include "Types.h"

namespace DTI
{
    class IDesignGenerator
    {
    public:
        virtual ~IDesignGenerator() = default;
        virtual void deploy(const DesignAssets& designData) = 0;
    };
}

#endif // I_DESIGN_GENERATOR_H
