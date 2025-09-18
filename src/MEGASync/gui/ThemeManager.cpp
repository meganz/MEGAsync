#include "ThemeManager.h"

#include "Platform.h"
#include "Preferences/Preferences.h"
#include "ServiceUrls.h"
#include "TextDecorator.h"
#include "Utilities.h"

const QMap<Preferences::ThemeType, QString> ThemeManager::mThemesMap = {
    {Preferences::ThemeType::SYSTEM_DEFAULT, QLatin1String("System default")},
    {Preferences::ThemeType::LIGHT_THEME, QLatin1String("Light")},
    {Preferences::ThemeType::DARK_THEME, QLatin1String("Dark")}};

ThemeManager::ThemeManager():
    QObject(nullptr),
    mCurrentTheme(Preferences::ThemeType::LAST),
    mCurrentColorScheme(Preferences::ThemeType::LAST)
{}

QString ThemeManager::getSelectedColorSchemaString() const
{
    return getColorSchemaString(mCurrentColorScheme);
}

QString ThemeManager::getColorSchemaString(Preferences::ThemeType themeId) const
{
    return mThemesMap.value(themeId, QLatin1String("Light"));
}

ThemeManager* ThemeManager::instance()
{
    static ThemeManager manager;

    return &manager;
}

void ThemeManager::init()
{
    setTheme(Preferences::instance()->getThemeType());
}

QStringList ThemeManager::themesAvailable() const
{
    return mThemesMap.values();
}

void ThemeManager::setTheme(Preferences::ThemeType theme)
{
    mCurrentTheme = theme;

    if (theme == Preferences::ThemeType::SYSTEM_DEFAULT)
    {
        auto soTheme = Platform::getInstance()->getCurrentTheme();
        applyTheme(soTheme);

        Platform::getInstance()->startThemeMonitor();
        connect(Platform::getInstance(),
                &AbstractPlatform::themeChanged,
                this,
                &ThemeManager::onOperatingSystemThemeChanged);
    }
    else
    {
        Platform::getInstance()->stopThemeMonitor();
        disconnect(Platform::getInstance(),
                   &AbstractPlatform::themeChanged,
                   this,
                   &ThemeManager::onOperatingSystemThemeChanged);

        applyTheme(theme);
    }

    Preferences::instance()->setThemeType(mCurrentTheme);
}

void ThemeManager::onOperatingSystemThemeChanged()
{
    applyTheme(Platform::getInstance()->getCurrentTheme());
}

void ThemeManager::applyTheme(Preferences::ThemeType theme)
{
    if (mCurrentColorScheme != theme)
    {
        mCurrentColorScheme = theme;

        if (!Platform::getInstance()->loadThemeResource(getColorSchemaString(theme)))
        {
            mega::MegaApi::log(mega::MegaApi::LOG_LEVEL_ERROR, "Error loading resource files.");
            auto desktopAppInstallerUrl = ServiceUrls::getDesktopAppUrl();
            auto title = QCoreApplication::translate("MegaError", "Alert");
            // URL handled through translations. TODO use placeholder
            auto msg = QCoreApplication::translate(
                "MegaError",
                "[B]Error detected[/B][BR]An error has occurred loading application resources, and "
                "the app needs to close. Please reinstall the app from [A]mega.io/desktop[/A] to "
                "resolve this issue. If the problem persists after reinstalling, contact support "
                "for further assistance.");
            Text::RichText(desktopAppInstallerUrl.toString()).process(msg);

            QMessageBox::warning(nullptr, title, msg, QMessageBox::Ok);
            ::exit(0);
        }

        emit themeChanged(theme);
        Utilities::propagateCustomEvent(ThemeChanged);
    }
}
