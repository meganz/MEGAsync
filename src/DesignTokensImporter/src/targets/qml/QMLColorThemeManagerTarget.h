#ifndef QML_COLOR_THEME_MANAGER_TARGET_H
#define QML_COLOR_THEME_MANAGER_TARGET_H

#include "DesignTarget.h"

#include <QString>

namespace DTI
{
    class QMLColorThemeManagerTarget : public IDesignTarget
    {
    public:
        //!
        //! \brief Creates the color manager that exposes all the color tokens available for the QML pages.
        //!
        //! This function has the responsability to create the color manager qml file with all the color tokens available for the QML pages.
        //! It knows where to place the file with the design tokens and is able to select the desired data from the repository.
        //!
        //! \param designAssets The design assets structure with all the data gathered from the design tokens.
        //!
        void deploy(const DesignAssets& designAssets) const override;

    private:
        static bool registered;

        bool checkThemeData(const ThemedColorData& themeData) const;
        void logColorTokensDifferences(const QString& themeName1, QStringList theme1ColorTokens, const QString& themeName2, QStringList theme2ColorTokens) const;
        void logMissingTokens(const QString& sourceTheme, const QString& targetTheme, QStringList::const_iterator begin, QStringList::const_iterator end) const;
    };
}

#endif
