#ifndef SNIPPING_TOOL_SETTINGS_H
#define SNIPPING_TOOL_SETTINGS_H

#include <QObject>
#include <QString>

#include <string>
#include <windows.h>

class QWinEventNotifier;

// Live view of the Snipping Tool's settings (its settings.dat), held open for
// the session so a relocation of its auto-save folder is announced the moment
// the tool writes it instead of being found by polling.
//
// RegLoadAppKey shares the hive the running tool has loaded rather than
// reading the file, so what is read here is current even while the file on
// disk lags behind. Holding it does not get in the tool's way: it starts and
// saves its settings as usual. RegNotifyChangeKeyValue signals an event on
// every value write under LocalState, and QWinEventNotifier delivers it on
// this object's thread.
class SnippingToolSettings: public QObject
{
    Q_OBJECT

public:
    explicit SnippingToolSettings(QObject* parent = nullptr);
    ~SnippingToolSettings() override;

    // FutureAccessList token naming the auto-save folder, or empty when the
    // user never relocated it or the settings cannot be opened (the tool is
    // not installed, or has never run). Opens the settings on first use and
    // reopens them when they were replaced, e.g. by resetting the app.
    QString autoSaveFolderToken();

    // True once the settings are open and watched. Until then a relocation is
    // not announced, so the caller has to ask again by itself.
    bool isWatching() const;

    // Arms toolExited() for the running Snipping Tool. Its private hive, which
    // names the folder a token points at, stays locked until the tool exits,
    // so that is the moment to look again. False when the tool is not running.
    bool watchToolExit();

    // The tool's package data folder (holds Settings and SystemAppData).
    static std::wstring packageRoot();

signals:
    // The token changed. Not emitted for the many other writes the tool makes
    // to its settings (window position, last tool used...).
    void autoSaveFolderTokenChanged();

    // The process watchToolExit() found has exited. Fires once per arming.
    void toolExited();

private slots:
    void onSettingsWritten();
    void onToolExited();

private:
    bool open();
    void close();
    bool watch();
    bool readToken(QString& token) const;

    HKEY mHive = nullptr;
    HKEY mLocalState = nullptr;
    // Manual reset: QWinEventNotifier leaves the event signalled, and
    // onSettingsWritten() resets it before arming the next notification.
    HANDLE mWrittenEvent = nullptr;
    QWinEventNotifier* mNotifier = nullptr;
    QString mToken;
    // A process handle is signalled once it exits; disarmed right after.
    HANDLE mToolProcess = nullptr;
    QWinEventNotifier* mToolExitNotifier = nullptr;
};

#endif // SNIPPING_TOOL_SETTINGS_H
