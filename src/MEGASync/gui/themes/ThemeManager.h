#ifndef THEMEMANAGER_H
#define THEMEMANAGER_H

#include "Preferences/Preferences.h"

#include <QObject>
#include <QString>

class ThemeManager : public QObject
{
    Q_OBJECT

public:
    static ThemeManager* instance();
    QStringList themesAvailable() const;
    Preferences::ThemeType getSelectedTheme() const;
    void setTheme(Preferences::ThemeType theme);

signals:
    void themeChanged(Preferences::ThemeType theme);

private:
    Preferences::ThemeType mCurrentTheme;
    static QMap<Preferences::ThemeType, QString> mThemesMap;

    ThemeManager();
};

#endif // THEMEMANAGER_H
