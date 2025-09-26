#pragma once
#include <QObject>

class MacThemeWatcher: public QObject
{
    Q_OBJECT

public:
    explicit MacThemeWatcher(QObject* parent = nullptr);
    ~MacThemeWatcher();

    bool getCurrentTheme() const;

signals:
    void systemThemeChanged(bool dark);

private:
    void* handle = nullptr;
    static void StaticCallback(bool isDark);
};
