#include "ThemeWidgetManager.h"

#include "themes/ThemeManager.h"

#include <QDir>
#include <QWidget>
#include <QtConcurrent/QtConcurrent>

static const QMap<Preferences::ThemeType, QString> themeNames = {
    {Preferences::ThemeType::LIGHT_THEME,  QObject::tr("Light")},
    {Preferences::ThemeType::DARK_THEME,  QObject::tr("Dark")}
};

static const QString jsonThemedColorFile = QLatin1String(":/colors/ColorThemedTokens.json");
static const QRegularExpression designTokensRE(QLatin1String("(#\\d*)\\/\\*colorToken\\.(.*)\\*\\/$"));

ThemeWidgetManager::ThemeWidgetManager(QObject *parent)
    : QObject{parent},
    mCurrentWidget{nullptr}
{
    connect(ThemeManager::instance(), &ThemeManager::themeChanged, this, &ThemeWidgetManager::onThemeChanged);

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

void ThemeWidgetManager::applyStyleSheet(QWidget* widget)
{
    if (widget == nullptr || widget == mCurrentWidget)
    //if (widget == nullptr)
    {
        return;
    }

    auto theme = ThemeManager::instance()->getSelectedTheme();
    auto currentTheme = themeToString(theme);

    if (!mColorThemedTokens.contains(currentTheme))
    {
        return;
    }

    mCurrentWidget = widget;

    const auto& themedColorTokens = mColorThemedTokens.value(currentTheme);

    QString styleSheet = widget->styleSheet();
    if (styleSheet.isEmpty())
    {
        return;
    }



    std::cout << "******************************************" << std::endl;
    const QString className = QString::fromUtf8(widget->metaObject()->className());
    std::cout << "Widget: " << className.toStdString() << std::endl;
    std::cout << "StyleSheet : " << styleSheet.toStdString() << std::endl;
    std::cout << std::endl;
    std::cout << std::endl;



    /*
    auto match = designTokensMatch.match(styleSheet);
    if (match.hasMatch())
    {
        auto value = match.capturedRef(0);
        QString token = match.captured(1);

        if (themedColorTokens.contains(token))
        {
            const QString& tokenValue = themedColorTokens.value(token);
            //value = QStringRef(tokenValue);
        }

        //match.lastCapturedIndex()
    }

    //styleSheet.replace()
    */

    QRegularExpression testDesignTokensRE(QLatin1String("#(\\d*)\\/\\*colorToken\\.(.*)\\*\\/"));
    QRegularExpressionMatchIterator matchIteratorTest = testDesignTokensRE.globalMatch(styleSheet);
    while (matchIteratorTest.hasNext()) {
        QRegularExpressionMatch match = matchIteratorTest.next();
        std::cout << "last capture index = " << match.lastCapturedIndex() << std::endl;

        for (int i = 0; i <= match.lastCapturedIndex(); ++i) {
            QString captured = match.captured(i);
            std::cout << "****************************************   --> "  << captured.toStdString() << std::endl;
        }
    }

    QRegularExpressionMatchIterator matchIterator = designTokensRE.globalMatch(styleSheet);
    while (matchIterator.hasNext()) {
        QRegularExpressionMatch match = matchIterator.next();
        std::cout << "last capture index = " << match.lastCapturedIndex() << std::endl;
    }


    /*
    QRegularExpressionMatch match = designTokensRE.match(styleSheet);
    if(match.hasMatch())
    {
        for (int i = 0; i <= match.lastCapturedIndex(); ++i) {
            QString captured = match.captured(i);
            std::cout << "****************************************"  << captured.toStdString() << std::endl;
        }

        widget->setStyleSheet(styleSheet);
    }
    */


}

std::shared_ptr<ThemeWidgetManager> ThemeWidgetManager::instance()
{
    static std::shared_ptr<ThemeWidgetManager> manager(new ThemeWidgetManager());
    return manager;
}

QString ThemeWidgetManager::themeToString(Preferences::ThemeType theme) const
{
    return themeNames.value(theme, QLatin1String("Light"));
}

void ThemeWidgetManager::onThemeChanged(Preferences::ThemeType theme)
{
    Q_UNUSED(theme)

    if (mCurrentWidget != nullptr)
    {
        applyStyleSheet(mCurrentWidget);
    }
}



