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
// Desktop; Windows: Pictures/Screenshots; Linux: Pictures and
// Pictures/Screenshots). When a new image file appears and its size has
// settled (the OS may still be writing it), captureDetected() is emitted.
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

signals:
    void captureDetected(const QString& filePath);

private slots:
    void onDirectoryChanged(const QString& path);
    void onStabilityTimerTimeout();

private:
    void scanDirectory(const QString& path);

    struct PendingFile
    {
        qint64 lastSize = -1;
        int stableTicks = 0;
    };

    QFileSystemWatcher mWatcher;
    QTimer mStabilityTimer;
    QHash<QString, PendingFile> mPendingFiles;
    QSet<QString> mKnownFiles;
    QDateTime mStartTime;
    bool mRunning = false;
};

#endif // SCREEN_CAPTURE_DETECTOR_H
