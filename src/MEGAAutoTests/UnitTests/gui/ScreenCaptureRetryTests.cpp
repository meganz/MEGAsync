#include "ScreenCaptureDetector.h"
#include <catch.hpp>

#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QTemporaryDir>
#include <QThread>

// Tests for the retry path: a capture directory the platform could not name at
// start() (on Windows the Snipping Tool keeps its settings hive locked while it
// runs) is attached once it resolves, and a capture taken in the meantime is
// still reported. The retry is driven through its slot rather than waited for,
// and the directories and the "pending" answer come from the test, so nothing
// here touches the user's real capture folders.

namespace
{

class LateDetector: public ScreenCaptureDetector
{
public:
    void setDirectories(const QStringList& directories, bool pending)
    {
        mDirectories = directories;
        mPending = pending;
    }

    void retry()
    {
        REQUIRE(QMetaObject::invokeMethod(this, "onRetryTimerTimeout"));
    }

    void tick(int times)
    {
        for (int i = 0; i < times; ++i)
        {
            REQUIRE(QMetaObject::invokeMethod(this, "onStabilityTimerTimeout"));
        }
    }

protected:
    QStringList captureDirectories() const override
    {
        return mDirectories;
    }

    bool captureDirectoriesPending() const override
    {
        return mPending;
    }

private:
    QStringList mDirectories;
    bool mPending = false;
};

class RetryFixture
{
protected:
    RetryFixture()
    {
        REQUIRE(mTempDir.isValid());
        mEarly = subdir(QString::fromUtf8("Screenshots"));
        mLate = subdir(QString::fromUtf8("Relocated"));
        QObject::connect(&mDetector,
                         &ScreenCaptureDetector::captureDetected,
                         [this](const QString& filePath)
                         {
                             mReported << filePath;
                         });
    }

    QString subdir(const QString& name) const
    {
        const QString path = QDir(mTempDir.path()).filePath(name);
        REQUIRE(QDir().mkpath(path));
        return path;
    }

    // See ScreenCaptureStabilityTests: keeps file timestamps clear of start().
    static void settleClock()
    {
        QThread::msleep(50);
    }

    QString write(const QString& directory, const QString& name) const
    {
        const QString path = QDir(directory).filePath(name);
        QFile file(path);
        REQUIRE(file.open(QIODevice::WriteOnly));
        REQUIRE(file.write(QByteArray(256, 'x')) == 256);
        file.close();
        return QDir::toNativeSeparators(QFileInfo(path).absoluteFilePath());
    }

    QTemporaryDir mTempDir;
    QString mEarly;
    QString mLate;
    LateDetector mDetector;
    QStringList mReported;
};

} // namespace

TEST_CASE_METHOD(RetryFixture, "a capture directory that resolves later is attached on retry")
{
    mDetector.setDirectories({mEarly}, true);
    mDetector.start();
    REQUIRE(mDetector.isRunning());
    settleClock();

    // Taken while the folder was still unknown to the detector: must not be
    // lost once the folder is.
    const QString capture = write(mLate, QString::fromUtf8("Screenshot 2026-10-06 154449.png"));

    mDetector.setDirectories({mEarly, mLate}, false);
    mDetector.retry();
    mDetector.tick(6);

    REQUIRE(mReported.size() == 1);
    CHECK(mReported.first() == capture);
}

TEST_CASE_METHOD(RetryFixture,
                 "files that predate start() are not reported when a directory is attached late")
{
    const QString old = write(mLate, QString::fromUtf8("Screenshot 2026-01-01 090000.png"));
    settleClock();

    mDetector.setDirectories({mEarly}, true);
    mDetector.start();
    settleClock();

    mDetector.setDirectories({mEarly, mLate}, false);
    mDetector.retry();
    mDetector.tick(6);

    CHECK(mReported.isEmpty());
    (void)old;
}

TEST_CASE_METHOD(RetryFixture, "a retry with nothing new to add is harmless")
{
    mDetector.setDirectories({mEarly}, true);
    mDetector.start();
    settleClock();

    mDetector.retry();
    mDetector.retry();
    mDetector.tick(2);

    CHECK(mReported.isEmpty());
    CHECK(mDetector.isRunning());

    // An unrelated image in a shared (not "Screenshots") folder attached late
    // still goes through the name filter.
    write(mLate, QString::fromUtf8("holiday.png"));
    mDetector.setDirectories({mEarly, mLate}, false);
    mDetector.retry();
    mDetector.tick(6);
    CHECK(mReported.isEmpty());
}
