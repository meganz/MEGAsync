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

static QRegularExpression colorThemeRE(QLatin1String("(#.*) *; *\\/\\* *colorToken\\.(.*)\\*\\/"));
static QRegularExpression imageThemeRE(QLatin1String(" *\\/\\* *ThemeImage;(.*);(.*);(.*);(.*);colorToken\\.(.*) *\\*\\/"));
static const QString jsonThemedColorFile = QLatin1String(":/colors/ColorThemedTokens.json");
static const QString themeToolButtonIcon = "ToolButtonIcon";

ThemeWidgetManager::ThemeWidgetManager(QObject *parent)
    : QObject{parent},
    mCurrentWidget{nullptr}
{
    connect(ThemeManager::instance(), &ThemeManager::themeChanged, this, &ThemeWidgetManager::onThemeChanged);

    colorThemeRE.optimize();
    imageThemeRE.optimize();

    loadColorThemeJson();
}

void ThemeWidgetManager::loadColorThemeJson()
{
    mColorThemedTokens.clear();

    QFile file(jsonThemedColorFile);
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
    QRegularExpressionMatchIterator matchIterator = colorThemeRE.globalMatch(styleSheet);
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

    matchIterator = imageThemeRE.globalMatch(styleSheet);
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

            changeImageColor(widget, mode, state, colorTokens, targetElementId, targetElementProperty, tokenId);
        }
    }

    if (updatedStyleSheet)
    {
        widget->setStyleSheet(styleSheet);
    }
}

void ThemeWidgetManager::changeImageColor(QWidget* widget, const QString& mode, const QString& state, const ColorTokens& colorTokens, const QString& targetElementId, const QString& targetElementProperty, const QString& tokenId)
{
    if(widget== nullptr || colorTokens.empty()|| targetElementId.isEmpty() || targetElementProperty.isEmpty() || tokenId.isEmpty())
    {
        return;
    }

    if (targetElementProperty == themeToolButtonIcon)
    {
        if (targetElementId == "bBackup")
        {
            std::cout << "backup found" << std::endl;
        }

        auto buttons = widget->findChildren<QToolButton*>(targetElementId);

        if (!buttons.isEmpty())
        {
            auto button = buttons.at(0);

            QIcon::State iconState = QIcon::On;
            if (state == "off")
            {
                iconState = QIcon::Off;
            }

            QIcon::Mode iconMode = QIcon::Normal;
            if (mode == "disabled")
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

            QIcon buttonIcons = button->icon();

            auto pixmap = buttonIcons.pixmap(button->iconSize());

            QColor toColor(colorTokens.value(tokenId));

            changePixmapColor(pixmap, toColor);

            buttonIcons.addPixmap(pixmap, iconMode, iconState);

            button->setIcon(buttonIcons);
        }
    }
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

    for(auto x=0; x<image.width(); x++)
    {
        for(auto y=0; y<image.height(); y++)
        {
            QColor color = image.pixelColor(x, y);
            toColor.setAlpha(color.alpha());
            image.setPixelColor(x, y, toColor);
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



