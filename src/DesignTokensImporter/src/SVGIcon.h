#ifndef SVGICON_H
#define SVGICON_H

#include "SVGRenderer.h"
#include "Types.h"
#include "utilities.h"

#include <QMap>
#include <QString>

namespace DTI
{
class SVGIcon
{
    enum class ButtonState
    {
        Normal,
        Pressed,
        Hovered
    };

    enum class StyleKey
    {
        Image,
        Icon,
        Width,
        Height,
        Colour,
        PressedColour,
        HoveredColour,
        None
    };

    struct ButtonStyle
    {
        ButtonState state;
        QString buttonStateName;
        QMap<StyleKey, QString> styleAttributes;
        QString name;
        Utilities::Theme theme;
    };

    struct SvgIconInfo
    {
        QString basePath;
        StyleKey GraphicType;
        QMap<QString, QString> properties;
        ButtonStyle currentButtonStyle;
    };

public:

    SVGIcon(const QString& uiFilename);

    bool processStylesheet(const QString& stylesheet);

    bool generateSVGImageBasedOnState();

    const ImageThemeStyleData& getImageStyle() const;

private:

    bool createAndSaveSVGImage(const ButtonStyle& buttonStyle, Utilities::Theme theme, QString& finalImageFilePath);
    bool saveSVGImage(const ButtonStyle& buttonStyle, const QString& colour, const QString& outputImageFilePath);
    QSize getSize() const;
    QString getColor(ButtonState state, Utilities::Theme theme) const;
    QString getButtonStateName(ButtonState state) const;
    QString extractImageName(const QString& resourcePath);
    QString getImagePathFromFileSystem(const QString& directoryPath, const QString& imageName);
    bool handleSvgCreationForTheme(const ButtonStyle& buttonStyle, Utilities::Theme theme, bool& bOk);
    void addSvgInfo(ButtonState state, const QMap<StyleKey, QString>& styleAttributes);
    QString styleKeyToString(const StyleKey& key) const;
    StyleKey stringToStyleKey(const QString& key) const;
    QString buttonStateToString(ButtonState buttonState) const;
    QString getColorKeyForState(ButtonState state) const;
    QString constructImageName(const QString& stateAsString);
    QString getImageDirectoryName();
    void addButtonStyle(const ButtonStyle& buttonStyle);
    const ImageThemeStyleData& getButtonStyles() const;

    SvgIconInfo mSvgIconInfo;
    const QString mUiFilename;
    SVGRenderer mSVGRenderer;
    ImageThemeStyleData mButtonStyles;
};
} // namespace DTI

#endif // SVGICON_H
