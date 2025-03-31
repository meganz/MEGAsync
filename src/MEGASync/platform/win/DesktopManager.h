#ifndef WIN_DESKTOP_MANAGER_H
#define WIN_DESKTOP_MANAGER_H

class DesktopManager
{
public:
    DesktopManager() = delete;
    ~DesktopManager() = delete;

    static void requestPinToTaskBarAsync();

private:
    static void requestPinToTaskBar();
    static bool IsDesktopAppPinningSupported();
    static bool TryUnlockPinningLaf();
};

#endif
