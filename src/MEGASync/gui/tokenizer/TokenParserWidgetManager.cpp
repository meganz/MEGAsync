#include "TokenParserWidgetManager.h"

#include "DialogOpener.h"
#include "IconTokenizer.h"
#include "MegaApplication.h"
#include "ThemeManager.h"

#include <QBitmap>
#include <QComboBox>
#include <QDir>
#include <QtConcurrent/QtConcurrent>
#include <QToolButton>
#include <QWidget>

namespace // anonymous namespace to hide names from other translation units
{
static QRegularExpression
    COLOR_TOKEN_REGULAR_EXPRESSION(QLatin1String("(#.*) *; *\\/\\* *colorToken\\.(.*)\\*\\/"));
static QRegularExpression ICON_COLOR_TOKEN_REGULAR_EXPRESSION(
    QLatin1String(" *\\/\\* *ColorTokenIcon;(.*);(.*);(.*);(.*);colorToken\\.(.*) *\\*\\/"));
static QRegularExpression REPLACE_THEME_TOKEN_REGULAR_EXPRESSION(
    QLatin1String(".*\\/(light|dark)\\/.*; *\\/\\* *replaceThemeToken *\\*\\/"));
static QRegularExpression
    SUB_WIDGET_WITH_STYLE_SHEET(QLatin1String(" *\\/\\* *SubWidget;(.*);(.*); *\\*\\/"));

static const QString JSON_THEMED_COLOR_TOKEN_FILE =
    QLatin1String(":/colors/ColorThemedTokens.json");
static const QString CSS_STANDARD_WIDGETS_COMPONENTS_FILE =
    QLatin1String(":/style/WidgetsComponentsStyleSheetsTokens.css");

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

enum SUB_WIDGET_INDEX_CAPTURE_INDEX
{
    SUB_WIDGET_INDEX_CAPTURE_WHOLE_MATCH,
    SUB_WIDGET_INDEX_CAPTURE_ID,
    SUB_WIDGET_INDEX_SUBWIDGET_TYPE
};
}

TokenParserWidgetManager::TokenParserWidgetManager(QObject* parent):
    QObject{parent}
{
    connect(ThemeManager::instance(),
            &ThemeManager::themeChanged,
            this,
            &TokenParserWidgetManager::onThemeChanged);
    connect(MegaSyncApp,
            &MegaApplication::updateUserInterface,
            this,
            &TokenParserWidgetManager::onUpdateRequested,
            Qt::QueuedConnection);

    COLOR_TOKEN_REGULAR_EXPRESSION.optimize();
    ICON_COLOR_TOKEN_REGULAR_EXPRESSION.optimize();

    loadColorThemeJson();
    loadStandardStyleSheetComponents();
}

void TokenParserWidgetManager::loadStandardStyleSheetComponents()
{
    mThemedStandardComponentsStyleSheet.clear();

    QFile file(CSS_STANDARD_WIDGETS_COMPONENTS_FILE);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text))
    {
        qWarning() << __func__ << " Error opening file : " << file.fileName();
        return;
    }

    QByteArray data = file.readAll();
    if (data.isEmpty())
    {
        qWarning() << __func__ << " Error reading file : " << file.fileName();
        return;
    }

    QString sourceStandardComponentsStyleSheet = QString::fromLatin1(data);
    for (const auto& theme: mColorThemedTokens.keys())
    {
        const auto& colorTokens = mColorThemedTokens.value(theme);

        replaceColorTokens(sourceStandardComponentsStyleSheet, colorTokens);
        replaceThemeTokens(sourceStandardComponentsStyleSheet, theme);
        mThemedStandardComponentsStyleSheet[theme] = sourceStandardComponentsStyleSheet;
    }
}

void TokenParserWidgetManager::loadColorThemeJson()
{
    mColorThemedTokens.clear();

    QFile file(JSON_THEMED_COLOR_TOKEN_FILE);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text))
    {
        qWarning() << __func__ << " Error opening file : " << file.fileName();
        return;
    }

    QByteArray jsonData = file.readAll();
    QJsonDocument jsonDoc = QJsonDocument::fromJson(jsonData);
    if (!jsonDoc.isObject())
    {
        qWarning() << __func__ << " Error invalid json format on file : " << file.fileName();
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

// performance mesurament code will be removed in latter stages of project.
void TokenParserWidgetManager::applyCurrentTheme(QWidget* dialog)
{
#if defined QT_DEBUG
    auto start = std::chrono::steady_clock::now();
#endif
    applyTheme(dialog);

#if defined QT_DEBUG
    auto end = std::chrono::steady_clock::now();
    std::chrono::duration<float> elapsed = end - start;

    qDebug() << "Time used to apply the theme : "
             << std::chrono::duration_cast<std::chrono::milliseconds>(elapsed).count() << " ms";
    qDebug() << "to the following dialog : " << dialog->objectName();
#endif
}

void TokenParserWidgetManager::registerWidgetForTheming(QWidget* dialog)
{
    QObject::connect(dialog,
                     &QWidget::destroyed,
                     this,
                     [this, dialog]()
                     {
                         mRegisteredWidgets.remove(dialog);
                     });
    mRegisteredWidgets.insert(dialog);
}

bool TokenParserWidgetManager::isTokenized(QWidget* widget)
{
    static QStringList tokenizedUiFiles =
        QString::fromUtf8(DESKTOP_APP_GUI_UI_FILES).split(QLatin1Char('|'));

    QString uiFileName = QLatin1String("ui/%0.ui").arg(widget->objectName());

    return !tokenizedUiFiles.filter(uiFileName).empty();
}

bool TokenParserWidgetManager::isRoot(QWidget* widget)
{
    static QStringList tokenizedRootUiFiles =
        QString::fromUtf8(DESKTOP_APP_GUI_UI_FILES_ROOT).split(QLatin1Char('|'));

    QString uiFileName = QLatin1String("ui/%0.ui").arg(widget->objectName());

    return !tokenizedRootUiFiles.filter(uiFileName).empty();
}

void TokenParserWidgetManager::applyCurrentTheme()
{
    QStringList dialogsName;
    foreach(const auto& dialog, DialogOpener::getAllOpenedDialogs())
    {
        if (!dialog.isNull())
        {
            applyTheme(dialog);

            dialogsName << dialog->objectName();
        }
    }

    for (auto widget: mRegisteredWidgets)
    {
        applyTheme(widget);
    }
}

QColor TokenParserWidgetManager::getColor(const QString& colorToken, const QString& defaultValue)
{
    QString color = defaultValue;

    auto currentTheme = ThemeManager::instance()->getSelectedThemeString();

    if (!mColorThemedTokens.contains(currentTheme))
    {
        qWarning() << __func__ << " Error theme not found : " << currentTheme;
    }
    else
    {
        const auto& colorTokens = mColorThemedTokens.value(currentTheme);
        if (!colorTokens.contains(colorToken))
        {
            qWarning() << __func__ << " Error color token not found : " << colorToken;
        }
        else
        {
            color = colorTokens.value(colorToken);
        }
    }

    return color;
}

void TokenParserWidgetManager::applyTheme(QWidget* widget, bool isSubWidget)
{
    if (!isSubWidget && !isTokenized(widget))
    {
        return;
    }

    auto currentTheme = ThemeManager::instance()->getSelectedThemeString();

    QString widgetStyleSheet;
    if (mWidgetsStyleSheets.contains(widget->objectName()))
    {
        widgetStyleSheet = mWidgetsStyleSheets[widget->objectName()];
    }
    else
    {
        widgetStyleSheet = widget->styleSheet();
        mWidgetsStyleSheets[widget->objectName()] = widgetStyleSheet;
    }

    if (!mColorThemedTokens.contains(currentTheme))
    {
        qWarning() << __func__ << " Error theme not found : " << currentTheme;
        return;
    }

    const auto& colorTokens = mColorThemedTokens.value(currentTheme);

    replaceColorTokens(widgetStyleSheet, colorTokens);
    replaceIconColorTokens(widget, widgetStyleSheet, colorTokens);
    replaceThemeTokens(widgetStyleSheet, currentTheme);
    tokenizeChildStyleSheets(widget, widgetStyleSheet);

    removeFrameOnDialogCombos(widget);

    QString styleSheet =
        (isRoot(widget) && !isSubWidget ? mThemedStandardComponentsStyleSheet[currentTheme] :
                                          QLatin1String()) %
        widgetStyleSheet;

    widget->setStyleSheet(styleSheet);
}

void TokenParserWidgetManager::removeFrameOnDialogCombos(QWidget* widget)
{
    auto comboBoxes = widget->findChildren<QComboBox*>();
    if (comboBoxes.isEmpty())
    {
        return;
    }

    for (auto comboBox: comboBoxes)
    {
        comboBox->view()->window()->setWindowFlags(Qt::Popup | Qt::FramelessWindowHint |
                                                   Qt::NoDropShadowWindowHint);
        comboBox->view()->window()->setAttribute(Qt::WA_TranslucentBackground);
    }
}

void TokenParserWidgetManager::replaceColorTokens(QString& styleSheet,
                                                  const ColorTokens& colorTokens)
{
    QRegularExpressionMatchIterator matchIterator =
        COLOR_TOKEN_REGULAR_EXPRESSION.globalMatch(styleSheet);
    while (matchIterator.hasNext())
    {
        QRegularExpressionMatch match = matchIterator.next();

        if (match.lastCapturedIndex() == COLOR_TOKEN_CAPTURE_INDEX::COLOR_DESIGN_TOKEN_NAME)
        {
            const QString& tokenValue = colorTokens.value(
                match.captured(COLOR_TOKEN_CAPTURE_INDEX::COLOR_DESIGN_TOKEN_NAME));

            auto startIndex = match.capturedStart(COLOR_TOKEN_CAPTURE_INDEX::COLOR_HEX_COLOR_VALUE);
            auto endIndex = match.capturedEnd(COLOR_TOKEN_CAPTURE_INDEX::COLOR_HEX_COLOR_VALUE);
            styleSheet.replace(startIndex, endIndex - startIndex, tokenValue);
        }
    }
}

void TokenParserWidgetManager::replaceIconColorTokens(QWidget* widget,
                                                      QString& styleSheet,
                                                      const ColorTokens& colorTokens)
{
    QRegularExpressionMatchIterator matchIterator =
        ICON_COLOR_TOKEN_REGULAR_EXPRESSION.globalMatch(styleSheet);
    while (matchIterator.hasNext())
    {
        QRegularExpressionMatch match = matchIterator.next();

        if (match.lastCapturedIndex() == ICON_TOKEN_CAPTURE_INDEX::ICON_TOKEN_DESIGN_TOKEN_NAME)
        {
            const QString& targetElementProperty =
                match.captured(ICON_TOKEN_CAPTURE_INDEX::ICON_TOKEN_TARGET_PROPERTY);
            const QString& targetElementId =
                match.captured(ICON_TOKEN_CAPTURE_INDEX::ICON_TOKEN_TARGET_ELEMENT_ID);
            const QString& mode = match.captured(ICON_TOKEN_CAPTURE_INDEX::ICON_TOKEN_TARGET_MODE);
            const QString& state =
                match.captured(ICON_TOKEN_CAPTURE_INDEX::ICON_TOKEN_TARGET_STATE);
            const QString& tokenId =
                match.captured(ICON_TOKEN_CAPTURE_INDEX::ICON_TOKEN_DESIGN_TOKEN_NAME);

            IconTokenizer::process(widget,
                                   mode,
                                   state,
                                   colorTokens,
                                   targetElementId,
                                   targetElementProperty,
                                   tokenId);
        }
    }
}

void TokenParserWidgetManager::replaceThemeTokens(QString& styleSheet, const QString& currentTheme)
{
    int adjustIndexOffset = 0;
    auto toReplaceTheme = currentTheme.toLower();
    auto toReplaceThemeLenght = toReplaceTheme.length();
    const QChar fillChar = QLatin1Char(' ');

    QRegularExpressionMatchIterator matchIterator =
        REPLACE_THEME_TOKEN_REGULAR_EXPRESSION.globalMatch(styleSheet);
    while (matchIterator.hasNext())
    {
        QRegularExpressionMatch match = matchIterator.next();

        if (match.lastCapturedIndex() ==
            REPLACE_THEME_TOKEN_CAPTURE_INDEX::REPLACE_THEME_TOKEN_THEME)
        {
            if (match.captured(REPLACE_THEME_TOKEN_CAPTURE_INDEX::REPLACE_THEME_TOKEN_THEME) ==
                toReplaceTheme)
            {
                continue;
            }

            auto startIndex =
                adjustIndexOffset +
                match.capturedStart(REPLACE_THEME_TOKEN_CAPTURE_INDEX::REPLACE_THEME_TOKEN_THEME);
            auto endIndex =
                adjustIndexOffset +
                match.capturedEnd(REPLACE_THEME_TOKEN_CAPTURE_INDEX::REPLACE_THEME_TOKEN_THEME);

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
        }
    }
}

std::shared_ptr<TokenParserWidgetManager> TokenParserWidgetManager::instance()
{
    static std::shared_ptr<TokenParserWidgetManager> manager(new TokenParserWidgetManager());
    return manager;
}

void TokenParserWidgetManager::onThemeChanged(Preferences::ThemeType theme)
{
    Q_UNUSED(theme)

    applyCurrentTheme();
}

void TokenParserWidgetManager::tokenizeChildStyleSheets(QWidget* widget, const QString& styleSheet)
{
    QRegularExpressionMatchIterator matchIterator =
        SUB_WIDGET_WITH_STYLE_SHEET.globalMatch(styleSheet);
    while (matchIterator.hasNext())
    {
        QRegularExpressionMatch match = matchIterator.next();

        if (match.lastCapturedIndex() ==
            SUB_WIDGET_INDEX_CAPTURE_INDEX::SUB_WIDGET_INDEX_SUBWIDGET_TYPE)
        {
            const QString& subWidgetId =
                match.captured(SUB_WIDGET_INDEX_CAPTURE_INDEX::SUB_WIDGET_INDEX_CAPTURE_ID);
            const QString& subWidgetType =
                match.captured(SUB_WIDGET_INDEX_CAPTURE_INDEX::SUB_WIDGET_INDEX_SUBWIDGET_TYPE);

            if (subWidgetType == QLatin1String("stack"))
            {
                auto stackedWidgets = widget->findChildren<QStackedWidget*>(subWidgetId);
                if (!stackedWidgets.empty())
                {
                    foreach(QStackedWidget* stackedWidget, stackedWidgets)
                    {
                        auto currentWidget = stackedWidget->currentWidget();
                        if (currentWidget != nullptr)
                        {
                            applyTheme(currentWidget, true);
                        }
                    }
                }
            }
            else if (subWidgetType == QLatin1String("widget"))
            {
                auto widgets = widget->findChildren<QWidget*>(subWidgetId);
                if (!widgets.empty())
                {
                    foreach(QWidget* subWidget, widgets)
                    {
                        if (subWidget != nullptr)
                        {
                            applyTheme(subWidget, true);
                        }
                    }
                }
            }
        }
    }
}
