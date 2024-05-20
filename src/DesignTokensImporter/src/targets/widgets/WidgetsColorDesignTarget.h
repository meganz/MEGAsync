#ifndef WIDGETS_COLOR_DESIGN_TARGET_H
#define WIDGETS_COLOR_DESIGN_TARGET_H

#include "DesignTarget.h"

#include "Types.h"

namespace DTI
{
    class WidgetsColorDesignTarget : public IDesignTarget
    {
    public:
        //!
        //! \brief Creates the file used by ThemeWidgetManager class to apply themed colors to widgets.
        //!
        //! This function has the responsability to create the file used by ThemeWidgetManager class to apply themed colors to widgets.
        //! It knows where to place the file with the design tokens and is able to select the desired data from the repository.
        //!
        //! \param designAssets The design assets structure with all the data gathered from the design tokens.
        //!
        void deploy(const DesignAssets& designAssets) const override;

    private:
        static bool registered;
    };
}

#endif
