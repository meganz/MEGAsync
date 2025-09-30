#pragma once
#include "Preferences.h"

#include <QObject>

class MacThemeWatcher: public QObject
{
    Q_OBJECT

public:
    explicit MacThemeWatcher(QObject* parent = nullptr);
    ~MacThemeWatcher();

    Preferences::ThemeAppeareance getCurrentTheme() const;

signals:
    void systemThemeChanged(Preferences::ThemeAppeareance);

private:
    void* handle = nullptr;
    static void StaticCallback(bool isDark);
};
