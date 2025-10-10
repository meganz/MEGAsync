#pragma once
#include "Preferences.h"

#include <QObject>

class MacThemeWatcher: public QObject
{
    Q_OBJECT

public:
    explicit MacThemeWatcher(QObject* parent = nullptr);
    ~MacThemeWatcher();

    Preferences::SystemColorScheme getCurrentTheme() const;

signals:
    void systemThemeChanged(Preferences::SystemColorScheme);

private:
    void* handle = nullptr;
    static void StaticCallback(bool isDark);
};
