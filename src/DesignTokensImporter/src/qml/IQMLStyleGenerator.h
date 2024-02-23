#ifndef I_QML_STYLE_GENERATOR_H
#define I_QML_STYLE_GENERATOR_H

#include "StyleDefinitions.h"

namespace DTI
{
    class IQMLStyleGenerator
    {
    public:
        virtual ~IQMLStyleGenerator() = default;

        virtual void start(const FilePathColourMap& styleData) = 0;
    };
}

#endif // I_QML_STYLE_GENERATOR_H
