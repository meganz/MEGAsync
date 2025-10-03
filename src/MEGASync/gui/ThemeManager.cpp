#include "ThemeManager.h"

#include "Platform.h"
#include "Preferences/Preferences.h"
#include "ServiceUrls.h"
#include "TextDecorator.h"
#include "Utilities.h"

const QMap<Preferences::ThemeAppeareance, QString> ThemeManager::mAppearance = {
    {Preferences::ThemeAppeareance::LIGHT, QLatin1String("Light")},
    {Preferences::ThemeAppeareance::DARK, QLatin1String("Dark")}};

inline Preferences::ThemeAppeareance toAppearance(Preferences::ThemeType type)
{
    switch (type)
    {
        case Preferences::ThemeType::DARK_THEME:
            return Preferences::ThemeAppeareance::DARK;
        case Preferences::ThemeType::LIGHT_THEME:
            return Preferences::ThemeAppeareance::LIGHT;
        case Preferences::ThemeType::SYSTEM_DEFAULT:
            [[fallthrough]];
        default:
            return Preferences::ThemeAppeareance::LIGHT;
    }
}

ThemeManager::ThemeManager():
    QObject(nullptr),
    mCurrentTheme(Preferences::ThemeType::UNINITIALIZED),
    mCurrentColorScheme(Preferences::ThemeAppeareance::UNINITIALIZED)
{
    connect(Platform::getInstance(),
            &AbstractPlatform::themeChanged,
            this,
            &ThemeManager::onSystemAppearanceChanged,
            Qt::UniqueConnection);
}

QString ThemeManager::getSelectedColorSchemaString() const
{
    return getColorSchemaString(mCurrentColorScheme);
}

QString ThemeManager::getColorSchemaString(Preferences::ThemeAppeareance themeId)
{
    return mAppearance.value(themeId, QLatin1String("Light"));
}

ThemeManager* ThemeManager::instance()
{
    static ThemeManager manager;

    return &manager;
}

void ThemeManager::init()
{
    auto theme = Preferences::instance()->getThemeType();
    const auto desktopTheme = Platform::getInstance()->getCurrentThemeAppearance().appsScheme;

    // If the desktop scheme is not available, fallback to Light theme
    if (desktopTheme == Preferences::ThemeAppeareance::UNINITIALIZED)
    {
        if (theme == Preferences::ThemeType::SYSTEM_DEFAULT ||
            theme == Preferences::ThemeType::UNINITIALIZED)
        {
            theme = Preferences::ThemeType::LIGHT_THEME;
        }
    }
    // If it is available and theme in Preferences is not initialized, use System Default
    else if (theme == Preferences::ThemeType::UNINITIALIZED)
    {
        theme = Preferences::ThemeType::SYSTEM_DEFAULT;
    }

    setTheme(theme);
}

Preferences::ThemeType ThemeManager::getCurrentTheme() const
{
    return mCurrentTheme;
}

Preferences::ThemeAppeareance ThemeManager::getCurrentColorScheme() const
{
    return mCurrentColorScheme;
}

QMap<Preferences::ThemeType, QString> ThemeManager::getAvailableThemes()
{
    mAvailableThemes.clear();

    if (Platform::getInstance()->getCurrentThemeAppearance().appsScheme !=
        Preferences::ThemeAppeareance::UNINITIALIZED)
    {
        mAvailableThemes.insert(Preferences::ThemeType::SYSTEM_DEFAULT, tr("System default"));
    }
    mAvailableThemes.insert(Preferences::ThemeType::DARK_THEME, tr("Dark"));
    mAvailableThemes.insert(Preferences::ThemeType::LIGHT_THEME, tr("Light"));

    return mAvailableThemes;
}

void ThemeManager::setTheme(Preferences::ThemeType theme)
{
    if (mCurrentTheme != theme)
    {
        mCurrentTheme = theme;

        if (theme == Preferences::ThemeType::SYSTEM_DEFAULT)
        {
            applyTheme(Platform::getInstance()->getCurrentThemeAppearance().appsScheme);
        }
        else
        {
            applyTheme(toAppearance(theme));
        }

        Preferences::instance()->setThemeType(mCurrentTheme);
    }
}

void ThemeManager::onSystemAppearanceChanged(const Preferences::SystemColorScheme& systemScheme)
{
    const auto appsScheme = systemScheme.appsScheme;
    if (mCurrentColorScheme != appsScheme &&
        appsScheme != Preferences::ThemeAppeareance::UNINITIALIZED &&
        (mCurrentTheme == Preferences::ThemeType::SYSTEM_DEFAULT ||
         Preferences::instance()->getThemeType() == Preferences::ThemeType::UNINITIALIZED))
    {
        applyTheme(appsScheme);
    }
}

void ThemeManager::applyTheme(Preferences::ThemeAppeareance theme)
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

        emit themeChanged();
        Utilities::propagateCustomEvent(ThemeChanged);
    }
}

Preferences::ThemeAppeareance ThemeManager::currentColorScheme() const
{
    return mCurrentColorScheme;
}
