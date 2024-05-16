#ifndef QML_COLOR_THEME_TARGET_H
#define QML_COLOR_THEME_TARGET_H

#include "DesignTarget.h"

#include <QString>

namespace DTI
{
    class QMLColorThemeTarget : public IDesignTarget
    {
    public:
        //!
        //! \brief Creates the files used by the QML ThemeManager to apply themed colors to qml pages.
        //!
        //! This function has the responsability to create the color qml file for every theme with the tokens received from the repository of design assets.
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
