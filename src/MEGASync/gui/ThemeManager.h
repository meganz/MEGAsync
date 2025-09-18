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
    QStringList themesAvailable() const;
    Preferences::ThemeType getSelectedTheme() const;
    QString getSelectedThemeString() const;
    QString getThemeString(Preferences::ThemeType theme) const;
    void setTheme(Preferences::ThemeType theme);
    void applyTheme(Preferences::ThemeType theme);

signals:
    void themeChanged(Preferences::ThemeType theme);

private:
    ThemeManager();
    void onOperatingSystemThemeChanged();

    Preferences::ThemeType mCurrentTheme;
    static const QMap<Preferences::ThemeType, QString> mThemesMap;
};

#endif
