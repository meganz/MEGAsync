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

QString ThemeManager::getColorSchemaString(Preferences::ThemeAppeareance themeId) const
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
    setTheme(Preferences::instance()->getThemeType());
}

QMap<Preferences::ThemeType, QString> ThemeManager::getAvailableThemes()
{
    mAvailableThemes.clear();

    if (Platform::getInstance()->getCurrentThemeAppearance() !=
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
            applyTheme(Platform::getInstance()->getCurrentThemeAppearance());
        }
        else
        {
            applyTheme(toAppearance(theme));
        }

        Preferences::instance()->setThemeType(mCurrentTheme);
    }
}

void ThemeManager::onSystemAppearanceChanged(Preferences::ThemeAppeareance app)
{
    if (mCurrentTheme == Preferences::ThemeType::SYSTEM_DEFAULT && mCurrentColorScheme != app)
    {
        applyTheme(app);
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
