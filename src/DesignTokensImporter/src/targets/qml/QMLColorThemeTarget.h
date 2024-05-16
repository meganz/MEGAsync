#ifndef QML_COLOR_THEME_TARGET_H
#define QML_COLOR_THEME_TARGET_H

#include "DesignTarget.h"

#include <QString>

namespace DTI
{
    class QMLColorThemeTarget : public IDesignTarget
    {
    public:
        void deploy(const DesignAssets& designAssets) const override;

    private:
        static bool registered;
    };
}

#endif
