#ifndef WIDGETS_COLOR_DESIGN_TARGET_H
#define WIDGETS_COLOR_DESIGN_TARGET_H

#include "WidgetsDesignTarget.h"

#include "Types.h"

namespace DTI
{
    class WidgetsColorDesignTarget : public IWidgetsDesignTarget
    {
    public:
        void process(const ThemedColorData& themedColorData) override;

    private:
        static bool registered;
    };
}

#endif
