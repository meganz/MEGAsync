#ifndef WIDGETS_COLOR_DESIGN_TARGET_H
#define WIDGETS_COLOR_DESIGN_TARGET_H

#include "WidgetsDesignTarget.h"

#include "Types.h"

namespace DTI
{
    class WidgetsColorDesignTarget : public IWidgetsDesignTarget
    {
    public:
        void deploy(const DesignAssets& designAssets) const override;

    private:
        static bool registered;
    };
}

#endif
