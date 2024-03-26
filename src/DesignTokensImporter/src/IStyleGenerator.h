#ifndef I_STYLE_GENERATOR_H
#define I_STYLE_GENERATOR_H

#include "Types.h"

namespace DTI
{
    class IStyleGenerator
    {
    public:
        virtual ~IStyleGenerator() = default;
        
        virtual void start(const ThemedColourMap& styleData) = 0;
    };
}

#endif // I_STYLE_GENERATOR_H
