#include "ScreenCaptureDetector.h"

#include "Platform.h"

#include <QDir>
#include <QFileInfo>
#include <QRegularExpression>
#include <QStandardPaths>

namespace
{
constexpr int STABILITY_INTERVAL_MS = 150;
constexpr int STABLE_TICKS_REQUIRED = 2; // size unchanged for ~300ms => fully written
const QStringList IMAGE_EXTENSIONS{QString::fromUtf8("png"),
                                   QString::fromUtf8("jpg"),
                                   QString::fromUtf8("jpeg")};
}

ScreenCaptureDetector::ScreenCaptureDetector(QObject* parent):
    QObject(parent)
{
    mStabilityTimer.setInterval(STABILITY_INTERVAL_MS);
    connect(&mWatcher,
            &QFileSystemWatcher::directoryChanged,
            this,
            &ScreenCaptureDetector::onDirectoryChanged);
    connect(&mStabilityTimer,
            &QTimer::timeout,
            this,
            &ScreenCaptureDetector::onStabilityTimerTimeout);
}

void ScreenCaptureDetector::start()
{
    if (mRunning)
    {
        return;
    }

    mStartTime = QDateTime::currentDateTime();
    mKnownFiles.clear();
    mPendingFiles.clear();
    mCaptureDirectories.clear();
    mPendingDirectories.clear();

    const QStringList directories = platformCaptureDirectories();
    for (const QString& directory: directories)
    {
        if (QDir(directory).exists())
        {
            watchCaptureDirectory(directory, true);
        }
        else
        {
            // The OS creates the capture folder on first use (e.g. Windows
            // creates Pictures/Screenshots with the first Win+PrtScn). Watch
            // the parent and attach the folder when it appears.
            mPendingDirectories.insert(directory);
            watchParentOf(directory);
        }
    }

    mRunning = !mWatcher.directories().isEmpty();
}

void ScreenCaptureDetector::stop()
{
    if (!mRunning)
    {
        return;
    }

    const QStringList watched = mWatcher.directories();
    if (!watched.isEmpty())
    {
        mWatcher.removePaths(watched);
    }
    mStabilityTimer.stop();
    mPendingFiles.clear();
    mKnownFiles.clear();
    mCaptureDirectories.clear();
    mPendingDirectories.clear();
    mRunning = false;
}

void ScreenCaptureDetector::watchCaptureDirectory(const QString& directory,
                                                  bool baselineExistingFiles)
{
    mWatcher.addPath(directory);
    mCaptureDirectories.insert(directory);

    // Baseline: everything already present is not a new capture. Skipped for
    // folders that appear while running (they are created by the OS together
    // with the first capture, which must not be swallowed); scanDirectory's
    // timestamp check decides there.
    if (baselineExistingFiles)
    {
        const QFileInfoList entries = QDir(directory).entryInfoList(QDir::Files | QDir::Hidden);
        for (const QFileInfo& info: entries)
        {
            mKnownFiles.insert(QDir::toNativeSeparators(info.absoluteFilePath()));
        }
    }
}

void ScreenCaptureDetector::watchParentOf(const QString& directory)
{
    const QString parent = QFileInfo(directory).absolutePath();
    if (QDir(parent).exists())
    {
        mWatcher.addPath(parent);
    }
}

void ScreenCaptureDetector::promotePendingDirectories()
{
    const auto pending = mPendingDirectories; // copy: mutated while iterating
    for (const QString& directory: pending)
    {
        if (QDir(directory).exists())
        {
            mPendingDirectories.remove(directory);
            watchCaptureDirectory(directory, false);
            scanDirectory(directory);
        }
    }
}

bool ScreenCaptureDetector::isRunning() const
{
    return mRunning;
}

QStringList ScreenCaptureDetector::platformCaptureDirectories()
{
    QStringList directories;
    directories << Platform::getInstance()->getScreenshotsLocation();
    directories.removeDuplicates();

    return directories;
}

bool ScreenCaptureDetector::looksLikeScreenshot(const QString& fileName)
{
    // macOS: "Screenshot 2026-06-10 at 14.05.33.png" (localized variants keep
    // the timestamp); Windows: "Screenshot (12).png"; GNOME: "Screenshot from
    // 2026-06-10 14-05-33.png". Match a known prefix ("captur" also covers
    // captura/capture d'écran) or a date followed by a time — a bare date
    // matches too many ordinary photo names.
    static const QRegularExpression pattern(
        QString::fromUtf8(
            "(screen\\s?shot|captur|\\d{4}-\\d{2}-\\d{2}\\D{0,10}\\d{1,2}[.:h\\-]\\d{2})"),
        QRegularExpression::CaseInsensitiveOption);
    return pattern.match(fileName).hasMatch();
}

bool ScreenCaptureDetector::isSharedDirectory(const QString& path)
{
    static const QStringList sharedRoots{
        QStandardPaths::writableLocation(QStandardPaths::DesktopLocation),
        QStandardPaths::writableLocation(QStandardPaths::PicturesLocation)};
    return sharedRoots.contains(path);
}

void ScreenCaptureDetector::onDirectoryChanged(const QString& path)
{
    promotePendingDirectories();

    if (!mCaptureDirectories.contains(path))
    {
        // A watched parent changed but the pending folder is still missing.
        return;
    }

    if (!QDir(path).exists())
    {
        // Deleted mid-session: QFileSystemWatcher drops the path silently.
        // Demote it so it re-attaches when the OS recreates it.
        mCaptureDirectories.remove(path);
        mPendingDirectories.insert(path);
        watchParentOf(path);
        return;
    }

    scanDirectory(path);
}

void ScreenCaptureDetector::scanDirectory(const QString& path)
{
    const QFileInfoList entries = QDir(path).entryInfoList(QDir::Files, QDir::Time);
    for (const QFileInfo& info: entries)
    {
        const QString filePath = QDir::toNativeSeparators(info.absoluteFilePath());

        if (mKnownFiles.contains(filePath) || mPendingFiles.contains(filePath))
        {
            continue;
        }

        // macOS writes a hidden ".Screenshot….png" first and renames it when
        // done; skip dot-files and wait for the final name to appear.
        if (info.fileName().startsWith(QLatin1Char('.')))
        {
            continue;
        }

        if (!IMAGE_EXTENSIONS.contains(info.suffix().toLower()) ||
            (isSharedDirectory(path) && !looksLikeScreenshot(info.fileName())))
        {
            mKnownFiles.insert(filePath);
            continue;
        }

        const QDateTime created =
            info.birthTime().isValid() ? info.birthTime() : info.lastModified();
        if (created < mStartTime)
        {
            mKnownFiles.insert(filePath);
            continue;
        }

        mPendingFiles.insert(filePath, PendingFile());
        if (!mStabilityTimer.isActive())
        {
            mStabilityTimer.start();
        }
    }
}

void ScreenCaptureDetector::onStabilityTimerTimeout()
{
    const QStringList pending = mPendingFiles.keys();
    for (const QString& filePath: pending)
    {
        const QFileInfo info(filePath);
        if (!info.exists())
        {
            mPendingFiles.remove(filePath);
            continue;
        }

        PendingFile& state = mPendingFiles[filePath];
        const qint64 size = info.size();
        if (size > 0 && size == state.lastSize)
        {
            if (++state.stableTicks >= STABLE_TICKS_REQUIRED)
            {
                mPendingFiles.remove(filePath);
                mKnownFiles.insert(filePath);
                emit captureDetected(filePath);
            }
        }
        else
        {
            state.lastSize = size;
            state.stableTicks = 0;
        }
    }

    if (mPendingFiles.isEmpty())
    {
        mStabilityTimer.stop();
    }
}
