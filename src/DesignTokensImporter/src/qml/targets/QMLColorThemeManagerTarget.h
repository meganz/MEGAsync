#ifndef QML_COLOR_THEME_MANAGER_TARGET_H
#define QML_COLOR_THEME_MANAGER_TARGET_H

#include "IQMLThemeTarget.h"

#include <QString>

namespace DTI
{
    class QMLColorThemeManagerTarget : public IQMLThemeTarget
    {
    public:
        void deploy(const ThemedColorData& themedColourMap) const override;

    private:
        static bool registered;

        bool checkThemeData(const ThemedColorData& themeData) const;
        void logColorTokensDifferences(const QString& themeName1, QStringList theme1ColorTokens, const QString& themeName2, QStringList theme2ColorTokens) const;
        void logMissingTokens(const QString& sourceTheme, const QString& targetTheme, QStringList::const_iterator begin, QStringList::const_iterator end) const;
    };
}

#endif
