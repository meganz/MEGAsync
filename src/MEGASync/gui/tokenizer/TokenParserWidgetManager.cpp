#include "TokenParserWidgetManager.h"

#include "ThemeManager.h"
#include "IconTokenizer.h"
#include "MegaApplication.h"
#include "DialogOpener.h"

#include <QDir>
#include <QWidget>
#include <QBitmap>
#include <QComboBox>
#include <QToolButton>

#include <QtConcurrent/QtConcurrent>

namespace // anonymous namespace to hide names from other translation units
{
    static const QMap<Preferences::ThemeType, QString> THEME_NAMES = {
        {Preferences::ThemeType::LIGHT_THEME,  QLatin1String("Light")},
        {Preferences::ThemeType::DARK_THEME,  QLatin1String("Dark")}
    };

    static QRegularExpression COLOR_TOKEN_REGULAR_EXPRESSION(QLatin1String("(#.*) *; *\\/\\* *colorToken\\.(.*)\\*\\/"));
    static QRegularExpression ICON_COLOR_TOKEN_REGULAR_EXPRESSION(QLatin1String(" *\\/\\* *ColorTokenIcon;(.*);(.*);(.*);(.*);colorToken\\.(.*) *\\*\\/"));
    static QRegularExpression REPLACE_THEME_TOKEN_REGULAR_EXPRESSION(QLatin1String(".*\\/(light|dark)\\/.*; *\\/\\* *replaceThemeToken *\\*\\/"));

    static const QString JSON_THEMED_COLOR_TOKEN_FILE = QLatin1String(":/colors/ColorThemedTokens.json");
    static const QString CSS_STANDARD_WIDGETS_COMPONENTS_FILE = QLatin1String(":/style/WidgetsComponentsStyleSheets.css");

    enum COLOR_TOKEN_CAPTURE_INDEX
    {
        COLOR_WHOLE_MATCH,
        COLOR_HEX_COLOR_VALUE,
        COLOR_DESIGN_TOKEN_NAME
    };

    enum ICON_TOKEN_CAPTURE_INDEX
    {
        ICON_TOKEN_WHOLE_MATCH,
        ICON_TOKEN_TARGET_PROPERTY,
        ICON_TOKEN_TARGET_ELEMENT_ID,
        ICON_TOKEN_TARGET_MODE,
        ICON_TOKEN_TARGET_STATE,
        ICON_TOKEN_DESIGN_TOKEN_NAME
    };

    enum REPLACE_THEME_TOKEN_CAPTURE_INDEX
    {
        REPLACE_THEME_TOKEN_WHOLE_MATCH,
        REPLACE_THEME_TOKEN_THEME
    };
}

TokenParserWidgetManager::TokenParserWidgetManager(QObject *parent)
    : QObject{parent}
{
    connect(ThemeManager::instance(), &ThemeManager::themeChanged, this, &TokenParserWidgetManager::onThemeChanged);
    connect(MegaSyncApp, &MegaApplication::updateUserInterface, this, &TokenParserWidgetManager::onUpdateRequested, Qt::QueuedConnection);

    COLOR_TOKEN_REGULAR_EXPRESSION.optimize();
    ICON_COLOR_TOKEN_REGULAR_EXPRESSION.optimize();

    loadColorThemeJson();
    loadStandardStyleSheetComponents();
}

void TokenParserWidgetManager::loadStandardStyleSheetComponents()
{
    mStandardComponentsStyleSheet.clear();

    QFile file(CSS_STANDARD_WIDGETS_COMPONENTS_FILE);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text))
    {
        qDebug() << __func__ << " Error opening file : " << file.fileName();
        return;
    }

    QByteArray data = file.readAll();
    if (data.isEmpty())
    {
        qDebug() << __func__ << " Error reading file : " << file.fileName();
        return;
    }

    mStandardComponentsStyleSheet = QLatin1String(data);
}

void TokenParserWidgetManager::loadColorThemeJson()
{
    mColorThemedTokens.clear();

    QFile file(JSON_THEMED_COLOR_TOKEN_FILE);
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

void TokenParserWidgetManager::onUpdateRequested()
{
    applyCurrentTheme();
}

void TokenParserWidgetManager::applyCurrentTheme()
{
#if DEBUG
    auto start = std::chrono::steady_clock::now();
#endif

    foreach(auto dialog, DialogOpener::getAllOpenedDialogs())
    {
        if (dialog)
        {
            applyTheme(dialog);
        }
    }

#if DEBUG
    auto end = std::chrono::steady_clock::now();
    std::chrono::duration<float> elapsed = end - start;
    std::cout << " TIME USED TO APPLY THEME : " << std::chrono::duration_cast<std::chrono::milliseconds>(elapsed).count() << " ms" << std::endl;
#endif
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

    const auto& colorTokens = mColorThemedTokens.value(currentTheme);

    QString styleSheet = mStandardComponentsStyleSheet % widget->styleSheet();

    bool updatedStyleSheet = false;

    updatedStyleSheet |= replaceColorTokens(styleSheet, colorTokens);
    updatedStyleSheet |= replaceIconColorTokens(widget, styleSheet, colorTokens);
    updatedStyleSheet |= replaceThemeTokens(styleSheet, currentTheme);

    removeFrameOnDialogCombos(widget);

    if (updatedStyleSheet)
    {
        widget->setStyleSheet(styleSheet);
    }
}

void TokenParserWidgetManager::removeFrameOnDialogCombos(QWidget* widget)
{
    auto comboBoxes = widget->findChildren<QComboBox*>();
    if (comboBoxes.isEmpty())
    {
        return;
    }

    for (auto comboBox : comboBoxes)
    {
        /*
         * WARNING HACK : this line will generate an error on widget, so the frame will be removed.
         * border-type is not a valid property.
        */
        comboBox->view()->window()->setStyleSheet(QLatin1String("border-type: none;"));

        comboBox->view()->window()->setWindowFlags(Qt::Popup | Qt::FramelessWindowHint | Qt::NoDropShadowWindowHint);
        comboBox->view()->window()->setAttribute(Qt::WA_TranslucentBackground);
    }
}

bool TokenParserWidgetManager::replaceColorTokens(QString& styleSheet, const ColorTokens& colorTokens)
{
    bool updated = false;

    QRegularExpressionMatchIterator matchIterator = COLOR_TOKEN_REGULAR_EXPRESSION.globalMatch(styleSheet);
    while (matchIterator.hasNext())
    {
        QRegularExpressionMatch match = matchIterator.next();

        if (match.lastCapturedIndex() == COLOR_TOKEN_CAPTURE_INDEX::COLOR_DESIGN_TOKEN_NAME)
        {
            const QString& tokenValue = colorTokens.value(match.captured(COLOR_TOKEN_CAPTURE_INDEX::COLOR_DESIGN_TOKEN_NAME));

            auto startIndex = match.capturedStart(COLOR_TOKEN_CAPTURE_INDEX::COLOR_HEX_COLOR_VALUE);
            auto endIndex = match.capturedEnd(COLOR_TOKEN_CAPTURE_INDEX::COLOR_HEX_COLOR_VALUE);
            styleSheet.replace(startIndex, endIndex-startIndex, tokenValue);
            updated = true;
        }
    }

    return updated;
}

bool TokenParserWidgetManager::replaceIconColorTokens(QWidget* widget, QString& styleSheet, const ColorTokens& colorTokens)
{
    bool updated = false;

    QRegularExpressionMatchIterator matchIterator = ICON_COLOR_TOKEN_REGULAR_EXPRESSION.globalMatch(styleSheet);
    while (matchIterator.hasNext())
    {
        QRegularExpressionMatch match = matchIterator.next();

        if (match.lastCapturedIndex() == ICON_TOKEN_CAPTURE_INDEX::ICON_TOKEN_DESIGN_TOKEN_NAME)
        {
            const QString& targetElementProperty = match.captured(ICON_TOKEN_CAPTURE_INDEX::ICON_TOKEN_TARGET_PROPERTY);
            const QString& targetElementId = match.captured(ICON_TOKEN_CAPTURE_INDEX::ICON_TOKEN_TARGET_ELEMENT_ID);
            const QString& mode = match.captured(ICON_TOKEN_CAPTURE_INDEX::ICON_TOKEN_TARGET_MODE);
            const QString& state = match.captured(ICON_TOKEN_CAPTURE_INDEX::ICON_TOKEN_TARGET_STATE);
            const QString& tokenId = match.captured(ICON_TOKEN_CAPTURE_INDEX::ICON_TOKEN_DESIGN_TOKEN_NAME);

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

    QRegularExpressionMatchIterator matchIterator = REPLACE_THEME_TOKEN_REGULAR_EXPRESSION.globalMatch(styleSheet);
    while (matchIterator.hasNext())
    {
        QRegularExpressionMatch match = matchIterator.next();

        if (match.lastCapturedIndex() == REPLACE_THEME_TOKEN_CAPTURE_INDEX::REPLACE_THEME_TOKEN_THEME)
        {
            if (match.captured(REPLACE_THEME_TOKEN_CAPTURE_INDEX::REPLACE_THEME_TOKEN_THEME) == toReplaceTheme)
            {
                continue;
            }

            auto startIndex = adjustIndexOffset + match.capturedStart(REPLACE_THEME_TOKEN_CAPTURE_INDEX::REPLACE_THEME_TOKEN_THEME);
            auto endIndex = adjustIndexOffset + match.capturedEnd(REPLACE_THEME_TOKEN_CAPTURE_INDEX::REPLACE_THEME_TOKEN_THEME);

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

    applyCurrentTheme();
}
