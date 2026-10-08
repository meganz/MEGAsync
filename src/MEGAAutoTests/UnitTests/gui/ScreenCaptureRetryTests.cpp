#include "Platform.h"
#include "ScreenCaptureDetector.h"
#include "TestSingletons.h"
#include <catch.hpp>

#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QTemporaryDir>
#include <QThread>

// Tests for the refresh path: the capture directories are resolved again when
// the platform announces a change, so a folder it could not name at start()
// (on Windows the Snipping Tool keeps its settings hive locked while it runs)
// is attached once it resolves, and a folder the user relocates the capture
// location to is picked up and the one left behind dropped. The refresh is
// driven through its slot, except where the platform signal itself is under
// test, and the directories and the "pending" answer come from the test, so
// nothing here touches the user's real capture folders.

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

    void refresh()
    {
        REQUIRE(QMetaObject::invokeMethod(this, "onCaptureDirectoriesChanged"));
    }

    // What QFileSystemWatcher would deliver for a change in the directory.
    void directoryChanged(const QString& directory)
    {
        REQUIRE(QMetaObject::invokeMethod(this, "onDirectoryChanged", Q_ARG(QString, directory)));
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
        mOther = subdir(QString::fromUtf8("RelocatedAgain"));
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
    QString mOther;
    LateDetector mDetector;
    QStringList mReported;
};

} // namespace

TEST_CASE_METHOD(RetryFixture, "a capture directory that resolves later is attached on refresh")
{
    mDetector.setDirectories({mEarly}, true);
    mDetector.start();
    REQUIRE(mDetector.isRunning());
    settleClock();

    // Taken while the folder was still unknown to the detector: must not be
    // lost once the folder is.
    const QString capture = write(mLate, QString::fromUtf8("Screenshot 2026-10-06 154449.png"));

    mDetector.setDirectories({mEarly, mLate}, false);
    mDetector.refresh();
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
    mDetector.refresh();
    mDetector.tick(6);

    CHECK(mReported.isEmpty());
    (void)old;
}

TEST_CASE_METHOD(RetryFixture, "a refresh with nothing new to add is harmless")
{
    mDetector.setDirectories({mEarly}, true);
    mDetector.start();
    settleClock();

    mDetector.refresh();
    mDetector.refresh();
    mDetector.tick(2);

    CHECK(mReported.isEmpty());
    CHECK(mDetector.isRunning());

    // An unrelated image in a shared (not "Screenshots") folder attached late
    // still goes through the name filter.
    write(mLate, QString::fromUtf8("holiday.png"));
    mDetector.setDirectories({mEarly, mLate}, false);
    mDetector.refresh();
    mDetector.tick(6);
    CHECK(mReported.isEmpty());
}

TEST_CASE_METHOD(RetryFixture, "a capture folder relocated while running is followed")
{
    mDetector.setDirectories({mEarly, mLate}, false);
    mDetector.start();
    settleClock();

    // The user picks another folder in the Snipping Tool, then captures.
    mDetector.setDirectories({mEarly, mOther}, false);
    mDetector.refresh();
    const QString capture = write(mOther, QString::fromUtf8("Screenshot 2026-10-07 121530.png"));
    mDetector.directoryChanged(mOther);
    mDetector.tick(6);

    REQUIRE(mReported.size() == 1);
    CHECK(mReported.first() == capture);

    // The folder moved away from is no longer a capture folder: what lands
    // there now is the user's own business.
    write(mLate, QString::fromUtf8("Screenshot 2026-10-07 121600.png"));
    mDetector.directoryChanged(mLate);
    mDetector.tick(6);
    CHECK(mReported.size() == 1);
}

TEST_CASE_METHOD(RetryFixture, "a capture settling in a folder moved away from is dropped")
{
    mDetector.setDirectories({mEarly, mLate}, false);
    mDetector.start();
    settleClock();

    write(mLate, QString::fromUtf8("Screenshot 2026-10-07 121530.png"));
    mDetector.directoryChanged(mLate);

    mDetector.setDirectories({mEarly}, false);
    mDetector.refresh();
    mDetector.tick(6);

    CHECK(mReported.isEmpty());
}

TEST_CASE_METHOD(RetryFixture, "an incomplete list does not drop a folder in use")
{
    mDetector.setDirectories({mEarly, mLate}, false);
    mDetector.start();
    settleClock();

    // The settings could not be read this pass (Windows: the Snipping Tool is
    // running and holds them), so the relocated folder is missing from the
    // answer -- while it is the very folder the tool is saving to.
    mDetector.setDirectories({mEarly}, true);
    mDetector.refresh();

    const QString capture = write(mLate, QString::fromUtf8("Screenshot 2026-10-07 121530.png"));
    mDetector.directoryChanged(mLate);
    mDetector.tick(6);

    REQUIRE(mReported.size() == 1);
    CHECK(mReported.first() == capture);
}

TEST_CASE_METHOD(RetryFixture, "a location change announced by the platform triggers a refresh")
{
    ensurePlatformCreated();
    AbstractPlatform* platform = Platform::getInstance();
    mDetector.setDirectories({mEarly}, false);
    mDetector.start();
    settleClock();

    // Taken in the new folder before the detector knows about it; the
    // announcement attaches the folder and finds the capture.
    mDetector.setDirectories({mEarly, mLate}, false);
    const QString capture = write(mLate, QString::fromUtf8("Screenshot 2026-10-07 131833.png"));
    emit platform->screenshotsLocationChanged();
    mDetector.tick(6);

    REQUIRE(mReported.size() == 1);
    CHECK(mReported.first() == capture);

    // Harmless once stopped: the detector no longer listens.
    mDetector.stop();
    mDetector.setDirectories({mEarly, mLate, mOther}, false);
    emit platform->screenshotsLocationChanged();
    CHECK_FALSE(mDetector.isRunning());
}
