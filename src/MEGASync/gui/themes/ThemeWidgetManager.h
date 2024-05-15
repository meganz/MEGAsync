#ifndef THEME_WIDGET_MANAGER_H
#define THEME_WIDGET_MANAGER_H

#include "Preferences/Preferences.h"

#include <QObject>

class ThemeWidgetManager : public QObject
{
    Q_OBJECT

public:
    static std::shared_ptr<ThemeWidgetManager> instance();

    void applyCurrentTheme(QWidget* widget);

private:
    explicit ThemeWidgetManager(QObject *parent = nullptr);
    QString themeToString(Preferences::ThemeType theme) const;
    void loadColorThemeJson();
    void onThemeChanged(Preferences::ThemeType theme);
    void applyTheme(QWidget* widget);

    QMap<QString, QMap<QString, QString>> mColorThemedTokens;
    QWidget* mCurrentWidget;
};

#endif // THEMEWIDGET_H
