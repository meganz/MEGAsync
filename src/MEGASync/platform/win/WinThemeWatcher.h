#ifndef WINTHEMEWATCHER_H
#define WINTHEMEWATCHER_H

#include "Preferences.h"

#include <QThread>

#include <windows.h>

class WinThemeWatcher: public QThread
{
    Q_OBJECT

public:
    explicit WinThemeWatcher(QObject* parent = nullptr);
    ~WinThemeWatcher() override;

    Preferences::SystemColorScheme getCurrentTheme() const;

signals:
    void systemThemeChanged(Preferences::SystemColorScheme);

protected:
    void run() override;

    void startWatching();
    void stopWatching();

private:
    volatile bool m_running;
    HANDLE m_stopEvent; // used to signal thread to quit immediately
};

#endif // WINTHEMEWATCHER_H
