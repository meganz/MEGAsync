#ifndef THEMEMANAGER_H
#define THEMEMANAGER_H

#include "Preferences/Preferences.h"

#include <QMap>
#include <QString>

class ThemeManager : public QObject
{
    Q_OBJECT

public:
    static ThemeManager* instance();
    QStringList themesAvailable() const;
    Preferences::Theme getSelectedTheme();
    void setTheme(Preferences::Theme newTheme);

private:
    ThemeManager();
    QString themeToString(Preferences::Theme theme) const;

signals:
    void themeChanged(Preferences::Theme newTheme);

private:
    Preferences::Theme mCurrentStyle;
    const QMap<Preferences::Theme, QString> mThemePaths;
    static QMap<Preferences::Theme, QString> mThemesMap;
};

#endif // THEMEMANAGER_H
