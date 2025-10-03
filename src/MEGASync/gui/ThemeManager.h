#ifndef THEME_MANAGER_H
#define THEME_MANAGER_H

#include "Preferences/Preferences.h"

#include <QObject>
#include <QString>

class ThemeManager : public QObject
{
    Q_OBJECT

public:
    inline static QEvent::Type ThemeChanged =
        static_cast<QEvent::Type>(QEvent::registerEventType());

    static ThemeManager* instance();
    void init();
    Preferences::ThemeType getCurrentTheme() const;
    QMap<Preferences::ThemeType, QString> getAvailableThemes();
    QString getSelectedColorSchemaString() const;
    static QString getColorSchemaString(Preferences::ThemeAppeareance theme);
    void setTheme(Preferences::ThemeType theme);
    void onSystemAppearanceChanged(const Preferences::SystemColorScheme& systemScheme);

    Preferences::ThemeAppeareance currentColorScheme() const;

signals:
    void themeChanged();

private:
    ThemeManager();
    void onOperatingSystemThemeChanged();
    void applyTheme(Preferences::ThemeAppeareance appearance);

    Preferences::ThemeType mCurrentTheme;
    Preferences::ThemeAppeareance mCurrentColorScheme;
    static const QMap<Preferences::ThemeAppeareance, QString> mAppearance;
    QMap<Preferences::ThemeType, QString> mAvailableThemes;
};

#endif
