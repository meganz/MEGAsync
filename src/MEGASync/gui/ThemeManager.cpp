#include "ThemeManager.h"

#include "Platform.h"
#include "Preferences/Preferences.h"
#include "TextDecorator.h"
#include "Utilities.h"

const QMap<Preferences::ThemeType, QString> ThemeManager::mThemesMap = {
    {Preferences::ThemeType::LIGHT_THEME,  QLatin1String("Light")},
    {Preferences::ThemeType::DARK_THEME,  QLatin1String("Dark")}
};

ThemeManager::ThemeManager()
    : QObject(nullptr)
{
    setTheme(Preferences::instance()->getThemeType());
}

Preferences::ThemeType ThemeManager::getSelectedTheme() const
{
    return mCurrentTheme;
}

QString ThemeManager::getSelectedThemeString() const
{
    return mThemesMap.value(mCurrentTheme, QLatin1String("Light"));
}

QString ThemeManager::getThemeString(Preferences::ThemeType type) const
{
    return mThemesMap.value(type, QLatin1String("Light"));
}

ThemeManager* ThemeManager::instance()
{
    static ThemeManager manager;

    return &manager;
}

QStringList ThemeManager::themesAvailable() const
{
    return mThemesMap.values();
}

void ThemeManager::setTheme(Preferences::ThemeType theme)
{
    if (mCurrentTheme != theme)
    {
        mCurrentTheme = theme;

        if (!Platform::getInstance()->loadThemeResource(getThemeString(theme)))
        {
            mega::MegaApi::log(mega::MegaApi::LOG_LEVEL_ERROR, "Error loading resource files.");

            auto title = QCoreApplication::translate("MegaError", "Alert");
            auto msg = QCoreApplication::translate(
                "MegaError",
                "[B]Error detected[/B][BR]An error has occurred loading application "
                "resources, and the app needs to close. If this happens more than once, reinstall "
                "the "
                "app from [A]mega.io/desktop[/A] or contact support for further assistance.");

            const Text::Bold boldDecorator;
            boldDecorator.process(msg);

            const Text::NewLine newLineDecorator;
            newLineDecorator.process(msg);

            const Text::Link linkDecorator(Utilities::DESKTOP_APP_URL);
            linkDecorator.process(msg);

            QMessageBox::warning(nullptr, title, msg, QMessageBox::Ok);
            ::exit(0);
        }

        Preferences::instance()->setThemeType(mCurrentTheme);

        emit themeChanged(theme);
        Utilities::propagateCustomEvent(ThemeChanged);
    }
}

