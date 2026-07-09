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

    const QStringList directories = platformCaptureDirectories();
    for (const QString& directory: directories)
    {
        if (QDir(directory).exists())
        {
            mWatcher.addPath(directory);

            // Baseline: everything already present is not a new capture.
            const QFileInfoList entries = QDir(directory).entryInfoList(QDir::Files | QDir::Hidden);
            for (const QFileInfo& info: entries)
            {
                mKnownFiles.insert(info.absoluteFilePath());
            }
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
    mRunning = false;
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
    // 2026-06-10 14-05-33.png". Match either a known prefix or a timestamp.
    static const QRegularExpression pattern(
        QString::fromUtf8("(screen\\s?shot|capture|\\d{4}-\\d{2}-\\d{2})"),
        QRegularExpression::CaseInsensitiveOption);
    return pattern.match(fileName).hasMatch();
}

void ScreenCaptureDetector::onDirectoryChanged(const QString& path)
{
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
            !looksLikeScreenshot(info.fileName()))
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
