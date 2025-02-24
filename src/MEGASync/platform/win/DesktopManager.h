#ifndef WIN_DESKTOP_MANAGER_H
#define WIN_DESKTOP_MANAGER_H

class DesktopManager
{
public:
    DesktopManager() = delete;
    ~DesktopManager() = delete;

    static bool requestPinToTaskBar();

private:
    static bool IsDesktopAppPinningSupported();
    static bool TryUnlockPinningLaf();
};

#endif
