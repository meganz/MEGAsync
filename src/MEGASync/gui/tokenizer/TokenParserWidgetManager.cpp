#include "TokenParserWidgetManager.h"

#include "DialogOpener.h"
#include "IconTokenizer.h"
#include "MegaApplication.h"
#include "ThemeManager.h"

#include <QBitmap>
#include <QComboBox>
#include <QDialogButtonBox>
#include <QDir>
#include <QLineEdit>
#include <QtConcurrent/QtConcurrent>
#include <QToolButton>

namespace // anonymous namespace to hide names from other translation units
{
static QRegularExpression
    COLOR_TOKEN_REGULAR_EXPRESSION(QLatin1String("(#.*) *; *\\/\\* *colorToken\\.(.*)\\*\\/"));
static QRegularExpression ICON_COLOR_TOKEN_REGULAR_EXPRESSION(
    QLatin1String(" *\\/\\* *ColorTokenIcon;(.*);(.*);(.*);(.*);colorToken\\.(.*) *\\*\\/"));

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
    if (!dialog || !isTokenized(dialog))
    {
        return;
    }

#if defined QT_DEBUG
    auto start = std::chrono::steady_clock::now();
#endif
    applyTheme(dialog);

#if defined QT_DEBUG
    auto end = std::chrono::steady_clock::now();
    std::chrono::duration<float> elapsed = end - start;

    qDebug() << "Time used to apply the theme : "
             << std::chrono::duration_cast<std::chrono::milliseconds>(elapsed).count() << " ms"
             << " to the following dialog : " << dialog->objectName();
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
    applyCurrentTheme(dialog);
}

void TokenParserWidgetManager::polish(QWidget* widget)
{
    widget->style()->unpolish(widget);
    widget->style()->polish(widget);
    widget->update();
}

bool TokenParserWidgetManager::isTokenized(QWidget* widget)
{
    return widget->property("TOKENIZED").toBool();
}

void TokenParserWidgetManager::applyCurrentTheme()
{
    foreach(const auto& dialog, DialogOpener::getAllOpenedDialogs())
    {
        applyCurrentTheme(dialog);
    }

    for (auto widget: qAsConst(mRegisteredWidgets))
    {
        applyCurrentTheme(widget);
    }
}

QColor TokenParserWidgetManager::getColor(const QString& colorToken)
{
    return getColor(colorToken, ThemeManager::instance()->getSelectedColorSchemaString());
}

QColor TokenParserWidgetManager::getColor(const QString& colorToken, const QString& colorSchema)
{
    QColor color;

    if (!mColorThemedTokens.contains(colorSchema))
    {
        qWarning() << __func__ << " Error theme not found : " << colorSchema;
    }
    else
    {
        const auto& colorTokens = mColorThemedTokens.value(colorSchema);
        color = QColor(colorTokens.value(colorToken, QString()));
        if (!color.isValid())
        {
            mega::MegaApi::log(mega::MegaApi::LOG_LEVEL_ERROR,
                               QString::fromUtf8("Color token not found: %1.")
                                   .arg(colorToken)
                                   .toUtf8()
                                   .constData());
            qWarning() << __func__ << " Error color token not found : " << colorToken;
            Q_ASSERT(false);
        }
    }

    return color;
}

void TokenParserWidgetManager::styleQFileDialog(QPointer<QFileDialog> dialog)
{
    if (dialog)
    {
        dialog->setProperty("TOKENIZED", true);
        const auto dimension = QLatin1String("small");
        auto* buttonBox = dialog->findChild<QDialogButtonBox*>(QLatin1String("buttonBox"));
        if (buttonBox)
        {
            auto applyButtonDesignProperties =
                [&dimension, buttonBox](QLatin1String designTypeValue,
                                        QDialogButtonBox::StandardButton buttonType)
            {
                auto* button = buttonBox->button(buttonType);
                if (button)
                {
                    button->setIcon({});
                    button->setProperty("type", designTypeValue);
                    button->setProperty("dimension", dimension);
                }
            };

            applyButtonDesignProperties(QLatin1String("primary"), QDialogButtonBox::Open);
            applyButtonDesignProperties(QLatin1String("primary"), QDialogButtonBox::Save);
            applyButtonDesignProperties(QLatin1String("secondary"), QDialogButtonBox::Cancel);
        }

        auto applyItemDesignProperties = [&dimension](auto itQWidget)
        {
            itQWidget->setProperty("type", QLatin1String("mega"));
            itQWidget->setProperty("dimension", dimension);
        };

        auto cbs = dialog->findChildren<QComboBox*>();
        std::for_each(cbs.cbegin(), cbs.cend(), applyItemDesignProperties);

        auto les = dialog->findChildren<QLineEdit*>();
        std::for_each(les.cbegin(), les.cend(), applyItemDesignProperties);
    }
}

void TokenParserWidgetManager::applyTheme(QWidget* widget)
{
    auto currentTheme = ThemeManager::instance()->getSelectedColorSchemaString();

    QString widgetThemedStyleSheet;
    if (mThemedWidgetStyleSheets.contains(currentTheme) &&
        mThemedWidgetStyleSheets[currentTheme].contains(widget->objectName()))
    {
        widgetThemedStyleSheet = mThemedWidgetStyleSheets[currentTheme][widget->objectName()];
        tokenizeChildStyleSheets(widget);
    }
    else
    {
        widgetThemedStyleSheet = helpApplyTheme(widget);
        mThemedWidgetStyleSheets[currentTheme][widget->objectName()] = widgetThemedStyleSheet;
    }

    if (!widgetThemedStyleSheet.isEmpty())
    {
        widget->setStyleSheet(widgetThemedStyleSheet);
    }
}

QString TokenParserWidgetManager::helpApplyTheme(QWidget* widget)
{
    auto currentTheme = ThemeManager::instance()->getSelectedColorSchemaString();

    static QMap<QString, QString> widgetsStyleSheets;

    QString widgetStyleSheet;
    if (widgetsStyleSheets.contains(widget->objectName()))
    {
        widgetStyleSheet = widgetsStyleSheets[widget->objectName()];
    }
    else
    {
        widgetStyleSheet = widget->styleSheet();
        widgetsStyleSheets[widget->objectName()] = widgetStyleSheet;
    }

    if (!mColorThemedTokens.contains(currentTheme))
    {
        qWarning() << __func__ << " Error theme not found : " << currentTheme;
        return {};
    }

    const auto& colorTokens = mColorThemedTokens.value(currentTheme);

    replaceColorTokens(widgetStyleSheet, colorTokens);
    replaceIconColorTokens(widget, widgetStyleSheet);
    tokenizeChildStyleSheets(widget);
    removeFrameOnDialogCombos(widget);

    QString styleSheet = (isTokenized(widget) ? mThemedStandardComponentsStyleSheet[currentTheme] :
                                                QLatin1String()) %
                         widgetStyleSheet;

    return styleSheet;
}

void TokenParserWidgetManager::removeFrameOnDialogCombos(QWidget* widget)
{
    auto comboBoxes = widget->findChildren<QComboBox*>();
    if (comboBoxes.isEmpty())
    {
        return;
    }

    for (auto comboBox: qAsConst(comboBoxes))
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

    QVector<QRegularExpressionMatch> matches;

    // Save in a container to do a reverse iteration later
    while (matchIterator.hasNext())
    {
        QRegularExpressionMatch match = matchIterator.next();
        matches.append(match);
    }

    // If we do a reverse iterator, we don´t affect the previous match with our changes
    for (auto match = matches.rbegin(); match != matches.rend(); ++match)
    {
        if (match->lastCapturedIndex() == COLOR_TOKEN_CAPTURE_INDEX::COLOR_DESIGN_TOKEN_NAME)
        {
            const QString& tokenValue = colorTokens.value(
                match->captured(COLOR_TOKEN_CAPTURE_INDEX::COLOR_DESIGN_TOKEN_NAME));

            auto startIndex =
                match->capturedStart(COLOR_TOKEN_CAPTURE_INDEX::COLOR_HEX_COLOR_VALUE);
            auto endIndex = match->capturedEnd(COLOR_TOKEN_CAPTURE_INDEX::COLOR_HEX_COLOR_VALUE);
            styleSheet.replace(startIndex, endIndex - startIndex, tokenValue);
        }
    }
}

void TokenParserWidgetManager::replaceIconColorTokens(QWidget* widget, QString& styleSheet)
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
                                   targetElementId,
                                   targetElementProperty,
                                   tokenId);
        }
    }
}

std::shared_ptr<TokenParserWidgetManager> TokenParserWidgetManager::instance()
{
    static std::shared_ptr<TokenParserWidgetManager> manager(new TokenParserWidgetManager());
    return manager;
}

void TokenParserWidgetManager::onThemeChanged()
{
    applyCurrentTheme();
}

void TokenParserWidgetManager::tokenizeChildStyleSheets(QWidget* widget)
{
    auto children = widget->findChildren<QWidget*>();
    for (const auto& child: qAsConst(children))
    {
        if (!child->styleSheet().isEmpty())
        {
            applyTheme(child);
        }
    }
}
