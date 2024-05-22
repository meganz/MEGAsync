#include "TokenParserWidgetManager.h"

#include "ThemeManager.h"
#include "IconTokenizer.h"

#include <QDir>
#include <QWidget>
#include <QToolButton>
#include <QBitmap>
#include <QtConcurrent/QtConcurrent>

static const QMap<Preferences::ThemeType, QString> THEME_NAMES = {
    {Preferences::ThemeType::LIGHT_THEME,  QObject::tr("Light")},
    {Preferences::ThemeType::DARK_THEME,  QObject::tr("Dark")}
};

static QRegularExpression colorTokenRegularExpression(QString::fromUtf8("(#.*) *; *\\/\\* *colorToken\\.(.*)\\*\\/"));
static QRegularExpression iconColorTokenRegularExpression(QString::fromUtf8(" *\\/\\* *ColorTokenIcon;(.*);(.*);(.*);(.*);colorToken\\.(.*) *\\*\\/"));
static const QString jsonThemedColorTokenFile = QString::fromUtf8(":/colors/ColorThemedTokens.json");

TokenParserWidgetManager::TokenParserWidgetManager(QObject *parent)
    : QObject{parent},
    mCurrentWidget{nullptr}
{
    connect(ThemeManager::instance(), &ThemeManager::themeChanged, this, &TokenParserWidgetManager::onThemeChanged);

    colorTokenRegularExpression.optimize();
    iconColorTokenRegularExpression.optimize();

    loadColorThemeJson();
}

void TokenParserWidgetManager::loadColorThemeJson()
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

void TokenParserWidgetManager::applyCurrentTheme(QWidget* widget)
{
    if (widget == nullptr || widget == mCurrentWidget || widget->styleSheet().isEmpty())
    {
        return;
    }

    mCurrentWidget = widget;

    applyTheme(widget);
}

void TokenParserWidgetManager::applyTheme(QWidget* widget)
{
    enum ColorTokenCaptureIndex
    {
        ColorWholeMatch,
        ColorHexColorValue,
        ColorDesignTokenName
    };

    enum IconTokenCaptureIndex
    {
        IconTokenWholeMatch,
        IconTokenTargetProperty,
        IconTokenTargetElementId,
        IconTokenTargetMode,
        IconTokenTargetState,
        IconTokenDesignTokenName
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

        if (match.lastCapturedIndex() == ColorTokenCaptureIndex::ColorDesignTokenName)
        {
            const QString& tokenValue = colorTokens.value(match.captured(ColorTokenCaptureIndex::ColorDesignTokenName));

            auto startIndex = match.capturedStart(ColorTokenCaptureIndex::ColorHexColorValue);
            auto endIndex = match.capturedEnd(ColorTokenCaptureIndex::ColorHexColorValue);
            styleSheet.replace(startIndex, endIndex-startIndex, tokenValue);
            updatedStyleSheet = true;
        }
    }

    matchIterator = iconColorTokenRegularExpression.globalMatch(styleSheet);
    while (matchIterator.hasNext())
    {
        QRegularExpressionMatch match = matchIterator.next();

        if (match.lastCapturedIndex() == IconTokenCaptureIndex::IconTokenDesignTokenName)
        {
            const QString& targetElementProperty = match.captured(IconTokenCaptureIndex::IconTokenTargetProperty);
            const QString& targetElementId = match.captured(IconTokenCaptureIndex::IconTokenTargetElementId);
            const QString& mode = match.captured(IconTokenCaptureIndex::IconTokenTargetMode);
            const QString& state = match.captured(IconTokenCaptureIndex::IconTokenTargetState);
            const QString& tokenId = match.captured(IconTokenCaptureIndex::IconTokenDesignTokenName);

            IconTokenizer::process(widget, mode, state, colorTokens, targetElementId, targetElementProperty, tokenId);
        }
    }

    if (updatedStyleSheet)
    {
        widget->setStyleSheet(styleSheet);
    }
}

std::shared_ptr<TokenParserWidgetManager> TokenParserWidgetManager::instance()
{
    static std::shared_ptr<TokenParserWidgetManager> manager(new TokenParserWidgetManager());
    return manager;
}

QString TokenParserWidgetManager::themeToString(Preferences::ThemeType theme) const
{
    return THEME_NAMES.value(theme, QLatin1String("Light"));
}

void TokenParserWidgetManager::onThemeChanged(Preferences::ThemeType theme)
{
    Q_UNUSED(theme)

    if (mCurrentWidget != nullptr)
    {
        applyTheme(mCurrentWidget);
    }
}
