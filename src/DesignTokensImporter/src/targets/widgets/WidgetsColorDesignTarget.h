#ifndef WIDGETS_COLOR_DESIGN_TARGET_H
#define WIDGETS_COLOR_DESIGN_TARGET_H

#include "DesignTarget.h"
#include "Types.h"

#include <QJsonObject>

namespace DTI
{
class WidgetsColorDesignTarget: public IDesignTarget
{
public:
    void deploy(const DesignAssets& designAssets) const override;

private:
    static bool registered;

    QJsonObject createThemeJson(const DesignAssets& designAssets) const;
    QJsonObject createColorJson(const ColorData& themeData) const;
    QString adjustColorValue(const QString& colorValue) const;
    void writeThemesToFile(const QJsonObject& jsonThemes) const;
};
}

#endif
