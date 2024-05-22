#include "ThemeWidgetManager.h"

#include "themes/ThemeManager.h"

#include <QDir>
#include <QWidget>
#include <QToolButton>
#include <QBitmap>
#include <QtConcurrent/QtConcurrent>

static const QMap<Preferences::ThemeType, QString> THEME_NAMES = {
    {Preferences::ThemeType::LIGHT_THEME,  QObject::tr("Light")},
    {Preferences::ThemeType::DARK_THEME,  QObject::tr("Dark")}
};

static QRegularExpression colorTokenRegularExpression(QLatin1String("(#.*) *; *\\/\\* *colorToken\\.(.*)\\*\\/"));
static QRegularExpression iconColorTokenRegularExpression(QLatin1String(" *\\/\\* *ColorTokenIcon;(.*);(.*);(.*);(.*);colorToken\\.(.*) *\\*\\/"));
static const QString jsonThemedColorTokenFile = QLatin1String(":/colors/ColorThemedTokens.json");
static const QString ToolButtonId = "ToolButton";

ThemeWidgetManager::ThemeWidgetManager(QObject *parent)
    : QObject{parent},
    mCurrentWidget{nullptr}
{
    connect(ThemeManager::instance(), &ThemeManager::themeChanged, this, &ThemeWidgetManager::onThemeChanged);

    colorTokenRegularExpression.optimize();
    iconColorTokenRegularExpression.optimize();

    loadColorThemeJson();
}

void ThemeWidgetManager::loadColorThemeJson()
{
    mColorThemedTokens.clear();

    QFile file(jsonThemedColorTokenFile);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text))
    {
        qDebug() << __func__ << " Error opening file : " << file.fileName();
        return;
    }

    QByteArray jsonData = file.readAll();
    QJsonDocument jsonDoc = QJsonDocument::fromJson(jsonData);
    if (!jsonDoc.isObject())
    {
        qDebug() << __func__ << " Error invalid json format on file : " << file.fileName();
        return;
    }

    QJsonObject rootObj = jsonDoc.object();
    for (auto it = rootObj.begin(); it != rootObj.end(); ++it)
    {
        QMap<QString, QString> tokens;

        QString theme = it.key();
        QJsonObject token = it.value().toObject();

        for (auto innerIt = token.begin(); innerIt != token.end(); ++innerIt)
        {
            tokens.insert(innerIt.key(), innerIt.value().toString());
        }

        mColorThemedTokens.insert(theme, tokens);
    }
}

void ThemeWidgetManager::applyCurrentTheme(QWidget* widget)
{
    if (widget == nullptr || widget == mCurrentWidget || widget->styleSheet().isEmpty())
    {
        return;
    }

    mCurrentWidget = widget;

    applyTheme(widget);
}

void ThemeWidgetManager::applyTheme(QWidget* widget)
{
    enum ColorThemeCaptureIndex
    {
        ColorWholeMatch = 0,
        ColorHexColorValue = 1,
        ColorDesignTokenName = 2
    };

    enum ImageThemeCaptureIndex
    {
        ImageWholeMatch = 0,
        ImageTargetProperty = 1,
        ImageTargetElementId = 2,
        ImageTargetMode = 3,
        ImageTargetState = 4,
        ImageDesignTokenName = 5
    };

    auto theme = ThemeManager::instance()->getSelectedTheme();
    auto currentTheme = themeToString(theme);

    if (!mColorThemedTokens.contains(currentTheme))
    {
        qDebug() << __func__ << " Error theme not found : " << currentTheme;
        return;
    }

    const auto& colorTokens = mColorThemedTokens.value(currentTheme);

    QString styleSheet = widget->styleSheet();

    bool updatedStyleSheet = false;
    QRegularExpressionMatchIterator matchIterator = colorTokenRegularExpression.globalMatch(styleSheet);
    while (matchIterator.hasNext())
    {
        QRegularExpressionMatch match = matchIterator.next();

        if (match.lastCapturedIndex() == ColorThemeCaptureIndex::ColorDesignTokenName)
        {
            const QString& tokenValue = colorTokens.value(match.captured(ColorThemeCaptureIndex::ColorDesignTokenName));

            auto startIndex = match.capturedStart(ColorThemeCaptureIndex::ColorHexColorValue);
            auto endIndex = match.capturedEnd(ColorThemeCaptureIndex::ColorHexColorValue);
            styleSheet.replace(startIndex, endIndex-startIndex, tokenValue);
            updatedStyleSheet = true;
        }
    }

    matchIterator = iconColorTokenRegularExpression.globalMatch(styleSheet);
    while (matchIterator.hasNext())
    {
        QRegularExpressionMatch match = matchIterator.next();

        if (match.lastCapturedIndex() == ImageThemeCaptureIndex::ImageDesignTokenName)
        {
            const QString& targetElementProperty = match.captured(ImageThemeCaptureIndex::ImageTargetProperty);
            const QString& targetElementId = match.captured(ImageThemeCaptureIndex::ImageTargetElementId);
            const QString& mode = match.captured(ImageThemeCaptureIndex::ImageTargetMode);
            const QString& state = match.captured(ImageThemeCaptureIndex::ImageTargetState);
            const QString& tokenId = match.captured(ImageThemeCaptureIndex::ImageDesignTokenName);

            changeIconColor(widget, mode, state, colorTokens, targetElementId, targetElementProperty, tokenId);
        }
    }

    if (updatedStyleSheet)
    {
        widget->setStyleSheet(styleSheet);
    }
}

void ThemeWidgetManager::changeIconColor(QWidget* widget, const QString& mode, const QString& state, const ColorTokens& colorTokens, const QString& targetElementId, const QString& targetElementProperty, const QString& tokenId)
{
    if (widget == nullptr|| mode.isEmpty() || state.isEmpty() || colorTokens.empty()|| targetElementId.isEmpty() || targetElementProperty.isEmpty() || tokenId.isEmpty())
    {
        qDebug() << __func__ << " Error on function arguments :"
                 << "\n widget is nullptr : " << QVariant(widget == nullptr).toString()
                 << "\n mode is empty : " << QVariant(mode.isEmpty()).toString()
                 << "\n state is empty : " << QVariant(state.isEmpty()).toString()
                 << "\n colorTokens are empty : " << QVariant(colorTokens.isEmpty()).toString()
                 << "\n targetElementId is empty : " << QVariant(targetElementId.isEmpty()).toString()
                 << "\n targetElementProperty is empty : " << QVariant(targetElementProperty.isEmpty()).toString()
                 << "\n tokenId is empty : " << QVariant(tokenId.isEmpty()).toString();

        return;
    }

    auto widgets = widget->findChildren<QWidget*>(targetElementId);
    if (widgets.isEmpty())
    {
        qDebug() << __func__ << " Error children widget not found for  : " << targetElementId;

        return;
    }

    if (targetElementProperty == ToolButtonId)
    {
        auto iconState = getIconState(state);
        if (!iconState.has_value())
        {
            // error log created on getIconState
            return;
        }

        auto iconMode = getIconMode(mode);
        if (!iconMode.has_value())
        {
            // error log created on getIconMode
            return;
        }

        auto button = dynamic_cast<QToolButton*>(widgets.constFirst());
        if (button == nullptr)
        {
            qDebug() << __func__ << " Error dynamic cast failed for Widget* to QToolButton* : " << targetElementId;
            return;
        }

        QIcon buttonIcons = button->icon();
        if (buttonIcons.isNull())
        {
            qDebug() << __func__ << " Error button icon is null : " << targetElementId;
            return;
        }

        auto pixmap = buttonIcons.pixmap(button->iconSize());
        if (pixmap.isNull())
        {
            qDebug() << __func__ << " Error default pixmap for icon is null : " << targetElementId;
            return;
        }

        if (!colorTokens.contains(tokenId))
        {
            qDebug() << __func__ << " Error token id not found : " << tokenId;
            return;
        }

        QColor toColor(colorTokens.value(tokenId));

        changePixmapColor(pixmap, toColor);

        buttonIcons.addPixmap(pixmap, iconMode.value(), iconState.value());

        button->setIcon(buttonIcons);
    }
}

std::optional<QIcon::Mode> ThemeWidgetManager::getIconMode(const QString& mode)
{
    std::optional<QIcon::Mode> iconMode;

    if (mode == "normal")
    {
        iconMode = QIcon::Normal;
    }
    else if (mode == "disabled")
    {
        iconMode = QIcon::Disabled;
    }
    else if (mode == "active")
    {
        iconMode =  QIcon::Active;
    }
    else if (mode == "selected")
    {
        iconMode = QIcon::Selected;
    }
    else
    {
        qDebug() << __func__ << " Error unknown icon mode : " << mode;
    }

    return iconMode;
}

std::optional<QIcon::State> ThemeWidgetManager::getIconState(const QString& state)
{
    std::optional<QIcon::State> iconState;

    if (state == "on")
    {
        iconState = QIcon::On;
    }
    else if (state == "off")
    {
        iconState = QIcon::Off;
    }
    else
    {
        qDebug() << __func__ << " Error unknown icon state : " << state;
    }

    return iconState;
}

void ThemeWidgetManager::changePixmapColor(QPixmap& pixmap, QColor toColor)
{
    if (pixmap.isNull())
    {
        return;
    }

    QImage image = pixmap.toImage();
    if (image.isNull())
    {
        return;
    }

    /*
     * we are using the requested color for every pixel on the image
     * only the alpha channel is preserved.
    */
    for(auto widthIndex = 0; widthIndex < image.width(); ++widthIndex)
    {
        for(auto heightIndex = 0; heightIndex < image.height(); ++heightIndex)
        {
            QColor color = image.pixelColor(widthIndex, heightIndex);
            toColor.setAlpha(color.alpha());
            image.setPixelColor(widthIndex, heightIndex, toColor);
        }
    }

    pixmap = QPixmap::fromImage(image);
}

std::shared_ptr<ThemeWidgetManager> ThemeWidgetManager::instance()
{
    static std::shared_ptr<ThemeWidgetManager> manager(new ThemeWidgetManager());
    return manager;
}

QString ThemeWidgetManager::themeToString(Preferences::ThemeType theme) const
{
    return THEME_NAMES.value(theme, QLatin1String("Light"));
}

void ThemeWidgetManager::onThemeChanged(Preferences::ThemeType theme)
{
    Q_UNUSED(theme)

    if (mCurrentWidget != nullptr)
    {
        applyTheme(mCurrentWidget);
    }
}



