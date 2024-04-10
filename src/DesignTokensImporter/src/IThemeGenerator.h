#ifndef I_THEME_GENERATOR_H
#define I_THEME_GENERATOR_H

#include "Types.h"

namespace DTI
{
    class IThemeGenerator
    {
    public:
        virtual ~IThemeGenerator() = default;
        
        virtual void start(const ThemedColourMap& styleData) = 0;
    };
}

#endif // I_STYLE_GENERATOR_H
