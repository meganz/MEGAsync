#include "SVGIcon.h"
#include "PathProvider.h"

#include <QDir>
#include <QIcon>
#include <QRegularExpression>
#include <QSize>
#include <QStringList>
#include <QtDebug>
#include <array>
#include <QStringBuilder>

namespace DTI
{
static const QString DEFAULT_ICON_COLOR = QStringLiteral("#ffff0000"); // RED Color
static constexpr int DEFAULT_ICON_WIDTH = 24;
static constexpr int DEFAULT_ICON_HEIGHT = 24;

SVGIcon::SVGIcon(const QString& uiFilename) :
    mUiFilename(uiFilename)
{

}

bool SVGIcon::processStylesheet(const QString& stylesheet)
{
    QStringList lines = stylesheet.split('\n');
    static QRegularExpression regex(R"(\s*([\w-]+)\s*:\s*([^;]+);)");
    static QRegularExpression svgPathRegex(R"(['"]([^'"]+\.svg)['"])");

    for (const QString& line : qAsConst(lines))
    {
        QRegularExpressionMatch match = regex.match(line);
        if (match.hasMatch() && match.lastCapturedIndex() == 2)
        {
            QString key = match.captured(1).toLower();
            QString value = match.captured(2).trimmed();

            if ((key == styleKeyToString(StyleKey::Image) || key == styleKeyToString(StyleKey::Icon))
                && value.contains(".svg"))
            {
                QRegularExpressionMatch svgMatch = svgPathRegex.match(value);
                if (svgMatch.hasMatch())
                {
                    mSvgIconInfo.basePath = svgMatch.captured(1);
                    mSvgIconInfo.GraphicType = stringToStyleKey(key);
                }
            }
            else
            {
                // Store other properties in a map
                mSvgIconInfo.properties[key] = value;
            }
        }
    }

    return !mSvgIconInfo.basePath.isEmpty();
}

bool SVGIcon::generateSVGImageBasedOnState()
{
    bool ret = true;
    static std::array<ButtonStyle, 3> ButtonStyles =
        {
            ButtonStyle{SVGIcon::ButtonState::Normal, getButtonStateName(SVGIcon::ButtonState::Normal)},
            ButtonStyle{SVGIcon::ButtonState::Pressed, getButtonStateName(SVGIcon::ButtonState::Pressed)},
            ButtonStyle{SVGIcon::ButtonState::Hovered, getButtonStateName(SVGIcon::ButtonState::Hovered)}
        };

    static std::array<Utilities::Theme, 2> themes = {Utilities::Theme::LIGHT, Utilities::Theme::DARK};
    for (auto theme : themes)
    {
        for (const auto& buttonStyle : ButtonStyles)
        {
            bool bOk = false;
            mSvgIconInfo.currentButtonStyle = buttonStyle;
            if(!handleSvgCreationForTheme(buttonStyle, theme, bOk))
            {
                if(!bOk)
                {
                    ret = false;
                }
            }
        }
    }

    return ret;
}

const ImageThemeStyleInfo& SVGIcon::getImageStyle() const
{
    return getButtonStyles();
}

bool SVGIcon::createAndSaveSVGImage(const ButtonStyle& buttonStyle, Utilities::Theme theme, QString& finalImageFilePath)
{
    const QString basePath = QDir::currentPath() % PathProvider::RELATIVE_SVG_PATH;
    const QString separator = QDir::separator();
    const QString themePath = Utilities::themeToString(theme);

    const QString fullPath = basePath % separator % getImageDirectoryName() % separator % themePath;

    // Create directory if it doesn't exist
    if (!Utilities::createDirectory(fullPath))
    {
        qDebug() << "Failed to create directory:" << fullPath;
        return false;
    }

    const QString imageName =  constructImageName(buttonStyle.buttonStateName);

    const QString outputImageFilePath = fullPath % separator % imageName;

    const QString colour = getColor(buttonStyle.state, theme);

    bool isSaveImageSuccess = saveSVGImage(buttonStyle, colour, outputImageFilePath);

    if(isSaveImageSuccess)
    {
        const QString imageFolderPath = PathProvider::RELATIVE_RESOURCE_FILE_IMAGES_PATH % separator % getImageDirectoryName();
        const QString imageFileResourcePath = imageFolderPath % separator % themePath % separator % imageName;
        const QString imagePath  = QDir::fromNativeSeparators(imageFileResourcePath);

        finalImageFilePath = "url(" % imagePath  % ")";

        return true;
    }
    else
    {
        return false;
    }
}

bool SVGIcon::saveSVGImage(const ButtonStyle& buttonStyle, const QString& colour, const QString& outputImageFilePath)
{
    QSize size = getSize();

    const QString directoryPath = QDir::currentPath() % PathProvider::RELATIVE_SVG_PATH % QDir::separator() % getImageDirectoryName();

    const QString sourceImagePath = getImagePathFromFileSystem(directoryPath, extractImageName(mSvgIconInfo.basePath));

    const QString svgData = mSVGRenderer.getSVGImageData(sourceImagePath, size, colour);

    if(svgData.isEmpty())
    {
        return false;
    }

    QFile file(outputImageFilePath);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text))
    {
        qDebug() << "Failed to open file for writing:" << outputImageFilePath;
        return false;
    }

    QTextStream out(&file);
    out << svgData;
    file.close();

    return true;
}

QSize SVGIcon::getSize() const
{
    int width = 0, height = 0;
    static const QRegularExpression regex("[^0-9]");
    if (mSvgIconInfo.properties.contains(styleKeyToString(StyleKey::Width)))
    {
        QString widthStr = mSvgIconInfo.properties.value(styleKeyToString(StyleKey::Width));
        widthStr.remove(regex); // Remove non-numeric characters
        width = widthStr.toInt();
    }

    if (mSvgIconInfo.properties.contains(styleKeyToString(StyleKey::Height)))
    {
        QString heightStr = mSvgIconInfo.properties.value(styleKeyToString(StyleKey::Height));
        heightStr.remove(regex);
        height = heightStr.toInt();
    }

    // Check if either width or height is zero and set default size to 24px
    if (width == 0 || height == 0)
    {
        width = DEFAULT_ICON_WIDTH;
        height = DEFAULT_ICON_HEIGHT;
    }


    return QSize(width, height);
}

QString SVGIcon::getColor(SVGIcon::ButtonState state, Utilities::Theme theme) const
{
    QMap<QString, QString> themeColourInfo =  Utilities::getColourMapInfo().value(theme);
    QString colorKey = getColorKeyForState(state);

    if (!colorKey.isEmpty() && mSvgIconInfo.properties.contains(colorKey))
    {
        QString colorToken = mSvgIconInfo.properties.value(colorKey);

        static QRegularExpression regex(R"(['"\{]*\s*([-\w]+)\s*['"\}]*;?)");
        QRegularExpressionMatch match = regex.match(colorToken);
        QString cleanedColorToken;
        if (match.hasMatch())
        {
            cleanedColorToken = match.captured(1).toLower();
        }

        QString colourValue =  Utilities::findValueByKey(themeColourInfo, cleanedColorToken);
        if(!colourValue.isEmpty())
        {
            return colourValue;
        }
    }

    return DEFAULT_ICON_COLOR; // Default color red
}

QString SVGIcon::getButtonStateName(ButtonState state) const
{
    static const QMap<ButtonState, QString> ButtonStateMap = {
        { ButtonState::Normal, "normalbutton" },
        { ButtonState::Pressed, "pressedbutton" },
        { ButtonState::Hovered, "hoveredbutton" }
    };

    return ButtonStateMap.value(state);
}

QString SVGIcon::extractImageName(const QString& resourcePath)
{
    return Utilities::extractFileName(resourcePath); // This will return "MIME.svg" for ":/images/svg/MIME.svg"
}

QString SVGIcon::getImagePathFromFileSystem(const QString& directoryPath, const QString& imageName)
{
    return QDir::fromNativeSeparators(directoryPath % "/" % imageName);
}

bool SVGIcon::handleSvgCreationForTheme(const ButtonStyle& buttonStyle, Utilities::Theme theme, bool& bOk)
{
    QString finalImageFilePath;

    // Check if the color for the specified button state is defined in the stylesheet
    QString colorKey = getColorKeyForState(buttonStyle.state);
    if(!mSvgIconInfo.properties.contains(colorKey))
    {
        bOk = true;
        return false;
    }

    bool success = createAndSaveSVGImage(buttonStyle, theme, finalImageFilePath);

    if (success)
    {
        mSvgIconInfo.currentButtonStyle.name = Utilities::extractFileNameNoExtension(finalImageFilePath);
        mSvgIconInfo.currentButtonStyle.theme = theme;
        mSvgIconInfo.currentButtonStyle.styleAttributes[mSvgIconInfo.GraphicType] = finalImageFilePath;
        mSvgIconInfo.currentButtonStyle.styleAttributes[StyleKey::Width] = mSvgIconInfo.properties.value(styleKeyToString(StyleKey::Width));
        mSvgIconInfo.currentButtonStyle.styleAttributes[StyleKey::Height] = mSvgIconInfo.properties.value(styleKeyToString(StyleKey::Width));

        addButtonStyle(mSvgIconInfo.currentButtonStyle);
    }

    return success;
}

QString SVGIcon::styleKeyToString(const StyleKey& key) const
{
    static const QMap<StyleKey, QString> styleMap = {
        {StyleKey::Image, "image"},
        {StyleKey::Icon, "qproperty-icon"},
        {StyleKey::Width, "width"},
        {StyleKey::Height, "height"},
        {StyleKey::Colour, "normal_color"},
        {StyleKey::PressedColour, "pressed_color"},
        {StyleKey::HoveredColour, "hover_color"}
    };

    return styleMap.value(key, "");
}

SVGIcon::StyleKey SVGIcon::stringToStyleKey(const QString& key) const
{
    if (key == styleKeyToString(StyleKey::Image))
        return StyleKey::Image;
    else if (key == styleKeyToString(StyleKey::Icon))
        return StyleKey::Icon;
    else if (key == styleKeyToString(StyleKey::Width))
        return StyleKey::Width;
    else if (key == styleKeyToString(StyleKey::Height))
        return StyleKey::Height;
    else if (key == styleKeyToString(StyleKey::Colour))
        return StyleKey::Colour;
    else if (key == styleKeyToString(StyleKey::PressedColour))
        return StyleKey::PressedColour;
    else if (key == styleKeyToString(StyleKey::HoveredColour))
        return StyleKey::HoveredColour;
    else
        return StyleKey::None;
}


QString SVGIcon::buttonStateToString(ButtonState buttonState) const
{
    static const QMap<ButtonState, QString> buttonStateMap = {
        {ButtonState::Normal, "Normal"},
        {ButtonState::Pressed, "Pressed"},
        {ButtonState::Hovered, "Hovered"}
    };

    return buttonStateMap.value(buttonState, "");
}

QString SVGIcon::getColorKeyForState(ButtonState state) const
{
    QString colorKey;
    switch (state)
    {
    case ButtonState::Normal:
        colorKey = styleKeyToString(StyleKey::Colour);
        break;
    case ButtonState::Pressed:
        colorKey = styleKeyToString(StyleKey::PressedColour);
        break;
    case ButtonState::Hovered:
        colorKey = styleKeyToString(StyleKey::HoveredColour);
        break;
    }
    return colorKey;
}

QString SVGIcon::constructImageName(const QString& stateAsString)
{
    const QString imageFileName = Utilities::extractFileNameNoExtension(mSvgIconInfo.basePath);
    const QString imageName = imageFileName % "_" % stateAsString % PathProvider::SVG_FILE_EXTENSION;
    return imageName;
}

QString SVGIcon::getImageDirectoryName()
{
    const QString urlString = QDir::fromNativeSeparators(mSvgIconInfo.basePath);

    QChar quoteChar = urlString.contains('\'') ? '\'' : '\"';

    // Extract the path
    int start = urlString.indexOf(quoteChar) + 1;
    int end = urlString.lastIndexOf(quoteChar);
    QString path = urlString.mid(start, end - start);

    // Find the folder name already available, if not folder name will be class name
    QFileInfo fileInfo(PathProvider::RELATIVE_SVG_PATH);
    const QString svgName = fileInfo.fileName();

    QStringList pathComponents = path.split('/');
    QString folderName;
    int svgIndex = pathComponents.indexOf(svgName);
    if (svgIndex != -1 && svgIndex + 1 < pathComponents.size())
    {
        folderName = pathComponents.at(svgIndex + 1);
    }

    if (!folderName.isEmpty())
    {
        const QString imageName = extractImageName(mSvgIconInfo.basePath);

        // Check if the folder name is the same as the image name
        if (folderName == imageName)
        {
            folderName = mUiFilename;
        }

        return folderName;
    }
    else
    {
        return mUiFilename; //Class Name is the Folder Name
    }
}

void SVGIcon::addButtonStyle(const ButtonStyle& buttonStyle)
{
    QString key = buttonStyle.name % ":" % Utilities::themeToString(buttonStyle.theme);  //key is in format -> imageName:theme
    QString buttonState = buttonStateToString(buttonStyle.state);

    QMap<QString, QString> styleAttributes;
    for (auto it = buttonStyle.styleAttributes.constBegin(); it != buttonStyle.styleAttributes.constEnd(); ++it)
    {
        styleAttributes[styleKeyToString(it.key())] = it.value();
    }

    ButtonStateStyleMap stateStyleMap;
    stateStyleMap[buttonState] = styleAttributes;

    mButtonStyles.insert(key, stateStyleMap);
}

const ImageThemeStyleInfo& SVGIcon::getButtonStyles() const
{
    return mButtonStyles;
}

} // namespace DTI
