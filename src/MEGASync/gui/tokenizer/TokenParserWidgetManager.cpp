#include "TokenParserWidgetManager.h"

#include "ThemeManager.h"
#include "IconTokenizer.h"
#include "MegaApplication.h"
#include "qcombobox.h"

#include <QDir>
#include <QWidget>
#include <QToolButton>
#include <QBitmap>
#include <QtConcurrent/QtConcurrent>

namespace // anonymous namespace to hide names from other translation units
{
    static const QMap<Preferences::ThemeType, QString> THEME_NAMES = {
        {Preferences::ThemeType::LIGHT_THEME,  QObject::tr("Light")},
        {Preferences::ThemeType::DARK_THEME,  QObject::tr("Dark")}
    };

    static QRegularExpression colorTokenRegularExpression(QString::fromUtf8("(#.*) *; *\\/\\* *colorToken\\.(.*)\\*\\/"));
    static QRegularExpression iconColorTokenRegularExpression(QString::fromUtf8(" *\\/\\* *ColorTokenIcon;(.*);(.*);(.*);(.*);colorToken\\.(.*) *\\*\\/"));
    static QRegularExpression replaceThemeTokenRegularExpression(QString::fromUtf8(".*\\/(light|dark)\\/.*; *\\/\\* *replaceThemeToken *\\*\\/"));

    static const QString jsonThemedColorTokenFile = QString::fromUtf8(":/colors/ColorThemedTokens.json");

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

    enum ReplaceThemeTokenCaptureIndex
    {
        ReplaceThemeTokenWholeMatch,
        ReplaceThemeTokenTheme
    };
}

TokenParserWidgetManager::TokenParserWidgetManager(QObject *parent)
    : QObject{parent},
    mCurrentWidget{nullptr}
{
    connect(ThemeManager::instance(), &ThemeManager::themeChanged, this, &TokenParserWidgetManager::onThemeChanged);
    connect(MegaSyncApp, &MegaApplication::updateUserInterface, this, &TokenParserWidgetManager::onUpdateRequested, Qt::QueuedConnection);

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

void TokenParserWidgetManager::onUpdateRequested()
{
    applyTheme(mCurrentWidget);
}

void TokenParserWidgetManager::applyTheme(QWidget* widget)
{
    auto theme = ThemeManager::instance()->getSelectedTheme();
    auto currentTheme = themeToString(theme);

    if (!mColorThemedTokens.contains(currentTheme))
    {
        qDebug() << __func__ << " Error theme not found : " << currentTheme;
        return;
    }

    auto start = std::chrono::high_resolution_clock::now();
    const auto& colorTokens = mColorThemedTokens.value(currentTheme);

    QString styleSheet = widget->styleSheet();

    bool updatedStyleSheet = false;

    updatedStyleSheet |= replaceColorTokens(styleSheet, colorTokens);
    updatedStyleSheet |= replaceIconColorTokens(widget, styleSheet, colorTokens);
    updatedStyleSheet |= replaceThemeTokens(styleSheet, currentTheme);

    if (updatedStyleSheet)
    {
        widget->setStyleSheet(styleSheet);
    }

    test(widget);

    auto end = std::chrono::high_resolution_clock::now();
    std::chrono::duration<float> elapsed = end - start;
    std::cout << "********************** " << " time to apply theme : " << elapsed.count() << " s " << std::chrono::duration_cast<std::chrono::milliseconds>(elapsed).count() << " ms" << std::endl;
}

void TokenParserWidgetManager::test(QWidget* widget)
{
    auto widgets = widget->findChildren<QComboBox*>(QLatin1String("cLanguage"));
    if (widgets.isEmpty())
    {
        std::cout << "node not found" << std::endl;
        return;
    }

    auto cLang = widgets.first();
    cLang->view()->window()->setStyleSheet(QLatin1String("border-type: none;"));
    cLang->view()->window()->setWindowFlags(Qt::Popup | Qt::FramelessWindowHint | Qt::NoDropShadowWindowHint);
    cLang->view()->window()->setAttribute(Qt::WA_TranslucentBackground);




    //foundWidget->setStyleSheet(QLatin1String("QGroupBox * { background-color: red; } "));
    //std::cout << "foundWidget : " << foundWidget->objectName().toStdString() << std::endl;

    //auto widgetParent = widgets.first()->parentWidget();

    //std::cout << "widgetParent : " << widgetParent->objectName().toStdString() << std::endl;

    //widgetParent->setStyleSheet(QLatin1String("QGroupBox * { background-color: red; } "));
}

bool TokenParserWidgetManager::replaceColorTokens(QString& styleSheet, const ColorTokens& colorTokens)
{
    bool updated = false;

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
            updated = true;
        }
    }

    return updated;
}

bool TokenParserWidgetManager::replaceIconColorTokens(QWidget* widget, QString& styleSheet, const ColorTokens& colorTokens)
{
    bool updated = false;

    QRegularExpressionMatchIterator matchIterator = iconColorTokenRegularExpression.globalMatch(styleSheet);
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

    return updated;
}

bool TokenParserWidgetManager::replaceThemeTokens(QString& styleSheet, const QString& currentTheme)
{
    bool updated = false;

    int adjustIndexOffset = 0;
    auto toReplaceTheme = currentTheme.toLower();
    auto toReplaceThemeLenght = toReplaceTheme.length();
    const QChar fillChar = QLatin1Char(' ');

    QRegularExpressionMatchIterator matchIterator = replaceThemeTokenRegularExpression.globalMatch(styleSheet);
    while (matchIterator.hasNext())
    {
        QRegularExpressionMatch match = matchIterator.next();

        if (match.lastCapturedIndex() == ReplaceThemeTokenCaptureIndex::ReplaceThemeTokenTheme)
        {
            if (match.captured(ReplaceThemeTokenCaptureIndex::ReplaceThemeTokenTheme) == toReplaceTheme)
            {
                continue;
            }

            auto startIndex = adjustIndexOffset + match.capturedStart(ReplaceThemeTokenCaptureIndex::ReplaceThemeTokenTheme);
            auto endIndex = adjustIndexOffset + match.capturedEnd(ReplaceThemeTokenCaptureIndex::ReplaceThemeTokenTheme);

            auto capturedLength = endIndex - startIndex;
            if (capturedLength > toReplaceThemeLenght)
            {
                // need to remove chars
                auto diff = capturedLength - toReplaceThemeLenght;
                styleSheet.remove(startIndex, diff);
                adjustIndexOffset -= diff;
            }
            else if (capturedLength < toReplaceThemeLenght)
            {
                // need to add chars
                auto diff = toReplaceThemeLenght - capturedLength;
                styleSheet.insert(startIndex, &fillChar, diff);
                adjustIndexOffset += diff;
            }

            styleSheet.replace(startIndex, toReplaceThemeLenght, toReplaceTheme);
            updated = true;
        }
    }

    return updated;
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
