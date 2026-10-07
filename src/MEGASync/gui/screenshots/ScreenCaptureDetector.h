#ifndef SCREEN_CAPTURE_DETECTOR_H
#define SCREEN_CAPTURE_DETECTOR_H

#include <QDateTime>
#include <QFileSystemWatcher>
#include <QHash>
#include <QObject>
#include <QSet>
#include <QStringList>
#include <QTimer>

// Capture hook: detects new screen captures by watching the OS screenshot
// directories (macOS: the `com.apple.screencapture location` default or the
// Desktop; Windows: Pictures/Screenshots; Linux: Pictures, Pictures/Screenshots
// and the GNOME capture folder under Pictures in the desktop language's
// spelling). When a new image file appears and its size has settled (the OS
// may still be writing it), captureDetected() is emitted.
//
// This is deliberately a directory watcher rather than a global-hotkey/screen
// grabber: it reuses the native capture UX (⇧⌘3/4, PrtScn, gnome-screenshot)
// and needs no extra OS permissions beyond file access.
class ScreenCaptureDetector: public QObject
{
    Q_OBJECT

public:
    explicit ScreenCaptureDetector(QObject* parent = nullptr);

    void start();
    void stop();
    bool isRunning() const;

    // Resolved per-platform; exposed for tests and for the settings UI.
    static QStringList platformCaptureDirectories();

    // True if the file name looks like an OS screenshot (used to ignore
    // unrelated images dropped into a watched folder, e.g. the Desktop).
    static bool looksLikeScreenshot(const QString& fileName);

    // Shared directories (Desktop, Pictures root, a capture location the user
    // relocated onto a folder of their own files) hold user files of all kinds,
    // so new images must also pass the name filter. Only a dedicated capture
    // folder (leaf name "Screenshots") accepts any new image, which is what
    // covers localized capture names. Anything unrecognized counts as shared:
    // uploading a user's unrelated files is far worse than missing a capture.
    static bool isSharedDirectory(const QString& path);

    // True if the directory is far wider than a capture folder (the home
    // directory itself, or a filesystem root) and must not be watched. The
    // capture location comes from user-controlled settings, so it can resolve
    // to either.
    static bool isTooBroadToWatch(const QString& directory);

signals:
    void captureDetected(const QString& filePath);

protected:
    // The directories start() will watch. Production always answers with
    // platformCaptureDirectories(); this exists so tests can point the detector
    // at a temporary folder instead of the user's real capture locations, which
    // they must never create files in.
    virtual QStringList captureDirectories() const;

    // True while captureDirectories() is known to be incomplete because a
    // user-configured location could not be read yet (Windows: the Snipping
    // Tool keeps its settings hive locked for as long as it runs). Folders
    // missing from an incomplete list are kept rather than dropped. Production
    // answers with the platform layer; tests override it.
    virtual bool captureDirectoriesPending() const;

private slots:
    void onDirectoryChanged(const QString& path);
    void onStabilityTimerTimeout();
    // Re-resolves captureDirectories(): attaches new folders, drops the ones
    // no longer listed. Connected to AbstractPlatform::screenshotsLocationChanged().
    void onCaptureDirectoriesChanged();

private:
    void scanDirectory(const QString& path);
    void addCaptureDirectory(const QString& directory, bool baselineExistingFiles);
    void removeCaptureDirectory(const QString& directory);
    void syncWatchedPaths();
    void watchCaptureDirectory(const QString& directory, bool baselineExistingFiles);
    void logWatchedDirectories() const;
    void watchParentOf(const QString& directory);
    void promotePendingDirectories();

    struct PendingFile
    {
        qint64 lastSize = -1;
        int stableTicks = 0;
        // Ticks spent waiting for the file to settle. Bounds the lifetime of an
        // entry that never does (e.g. a capture tool that created the file and
        // died before writing it), which would otherwise keep the stability
        // timer running for the rest of the session.
        int ticksWaited = 0;
    };

    QFileSystemWatcher mWatcher;
    QTimer mStabilityTimer;
    QMetaObject::Connection mLocationChangedConnection;
    QHash<QString, PendingFile> mPendingFiles;
    QSet<QString> mKnownFiles;
    // Directories actively scanned for captures. mWatcher may also hold
    // parents of mPendingDirectories (capture folders the OS has not created
    // yet); those are watched only to catch the folder's creation.
    QSet<QString> mCaptureDirectories;
    QSet<QString> mPendingDirectories;
    QDateTime mStartTime;
    bool mRunning = false;
    // Guards onStabilityTimerTimeout() against re-entering itself: emitting
    // captureDetected() runs arbitrary code that can spin a nested event loop,
    // which dispatches this same timer.
    bool mInStabilityTick = false;
};

#endif // SCREEN_CAPTURE_DETECTOR_H
