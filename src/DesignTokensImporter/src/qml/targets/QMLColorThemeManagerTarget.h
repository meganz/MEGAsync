#ifndef QML_COLOR_THEME_MANAGER_TARGET_H
#define QML_COLOR_THEME_MANAGER_TARGET_H

#include "QMLDesignTarget.h"

#include <QString>

namespace DTI
{
    class QMLColorThemeManagerTarget : public IQMLDesignTarget
    {
    public:
        void deploy(const DesignAssets& designAssets) const override;

    private:
        static bool registered;

        bool checkThemeData(const ThemedColorData& themeData) const;
        void logColorTokensDifferences(const QString& themeName1, QStringList theme1ColorTokens, const QString& themeName2, QStringList theme2ColorTokens) const;
        void logMissingTokens(const QString& sourceTheme, const QString& targetTheme, QStringList::const_iterator begin, QStringList::const_iterator end) const;
    };
}

#endif
