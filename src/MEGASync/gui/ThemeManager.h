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
    QString getSelectedColorSchemaString() const;
    QString getColorSchemaString(Preferences::ThemeAppeareance theme) const;
    void setTheme(Preferences::ThemeType theme);
    void onSystemAppearanceChanged(Preferences::ThemeAppeareance app);

signals:
    void themeChanged();

private:
    ThemeManager();
    void onOperatingSystemThemeChanged();
    void recomputeAndApply();

    void applyTheme(Preferences::ThemeAppeareance appearance);

    Preferences::ThemeType mCurrentTheme;
    Preferences::ThemeAppeareance mCurrentColorScheme;
    static const QMap<Preferences::ThemeAppeareance, QString> mAppearance;
    static const QStringList mThemesList;
};

#endif
