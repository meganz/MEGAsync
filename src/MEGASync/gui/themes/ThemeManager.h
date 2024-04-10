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
    Preferences::Theme getSelectedTheme() const;
    void setTheme(Preferences::Theme theme);

signals:
    void themeChanged(Preferences::Theme theme);

private:
    Preferences::Theme mCurrentTheme;
    static QMap<Preferences::Theme, QString> mThemesMap;

    ThemeManager();
};

#endif // THEMEMANAGER_H
