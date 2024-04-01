#ifndef QML_COLOR_THEME_TARGET_H
#define QML_COLOR_THEME_TARGET_H

#include "IQMLThemeTarget.h"

#include <QString>

namespace DTI
{
    class QMLColorThemeTarget : public IQMLThemeTarget
    {
    public:
        void deploy(const ThemedColourMap& themedColourMap) const override;

    private:
        static bool registered;
    };
}

#endif
