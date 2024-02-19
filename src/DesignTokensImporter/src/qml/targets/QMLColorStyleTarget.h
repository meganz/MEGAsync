#ifndef QML_COLOR_STYLE_TARGET_H
#define QML_COLOR_STYLE_TARGET_H

#include "IQMLStyleTarget.h"

#include <string>

namespace DTI
{
    class QMLColorStyleTarget : public IQMLStyleTarget
    {
    public:
        void deploy(QString theme, const ColourMap& colourMap) const override;

    private:
        QString normalizeTokenId(QString tokenId) const;
        static bool registered;
    };
}

#endif
