#include "ScreenCaptureDetector.h"
#include <catch.hpp>

#include <QDir>
#include <QEventLoop>
#include <QFile>
#include <QTemporaryDir>
#include <QThread>
#include <QTimer>

// Tests for the stability tick -- the loop that decides a newly appeared file has
// finished being written and is therefore a capture to upload.
//
// The property that matters is that captureDetected() is emitted synchronously
// from inside that loop, and its real handler runs arbitrary code:
// QmlDialogWrapper's constructor spins a QEventLoop while its QML loads, and
// EventUpdater calls QApplication::processEvents() on the ordinary upload path.
// Either one re-enters the tick. Reporting a capture twice means uploading it
// twice and, with link copying on, exporting two public links for it.
//
// The detector is driven against a QTemporaryDir through the captureDirectories()
// seam, so nothing here can touch the user's real capture folders. Ticks are
// invoked directly rather than waited for, which keeps these deterministic and
// fast; onStabilityTimerTimeout() is a private slot, reachable through the
// meta-object without widening its access in production code.

namespace
{

// Matches MAX_PENDING_TICKS in ScreenCaptureDetector.cpp, plus headroom. Declared
// here because that constant is deliberately internal to the detector.
constexpr int MORE_THAN_MAX_PENDING_TICKS = 210;

class TestableDetector: public ScreenCaptureDetector
{
public:
    explicit TestableDetector(const QString& directory):
        mDirectories(QStringList{directory})
    {}

    void tick()
    {
        REQUIRE(QMetaObject::invokeMethod(this, "onStabilityTimerTimeout"));
    }

    void tick(int times)
    {
        for (int i = 0; i < times; ++i)
        {
            tick();
        }
    }

    void rescan(const QString& directory)
    {
        REQUIRE(QMetaObject::invokeMethod(this, "onDirectoryChanged", Q_ARG(QString, directory)));
    }

protected:
    QStringList captureDirectories() const override
    {
        return mDirectories;
    }

private:
    QStringList mDirectories;
};

// Collects what the detector reported. A plain connection rather than
// QSignalSpy, which lives in Qt5Test and is not linked into this target.
class CaptureRecorder
{
public:
    explicit CaptureRecorder(ScreenCaptureDetector& detector)
    {
        QObject::connect(&detector,
                         &ScreenCaptureDetector::captureDetected,
                         [this](const QString& filePath)
                         {
                             mReported << filePath;
                             if (mOnCapture)
                             {
                                 mOnCapture();
                             }
                         });
    }

    // Runs inside the signal handler, standing in for whatever the real
    // controller does -- including spinning a nested event loop.
    void setOnCapture(std::function<void()> onCapture)
    {
        mOnCapture = std::move(onCapture);
    }

    int count() const
    {
        return mReported.size();
    }

    int countOf(const QString& filePath) const
    {
        return mReported.count(filePath);
    }

    QString first() const
    {
        return mReported.isEmpty() ? QString() : mReported.first();
    }

private:
    QStringList mReported;
    std::function<void()> mOnCapture;
};

class StabilityFixture
{
protected:
    StabilityFixture()
    {
        REQUIRE(mTempDir.isValid());
        // Named "Screenshots" so it counts as a dedicated capture folder: these
        // tests are about settling, not about the file-name filter, which has
        // its own tests.
        mWatched = QDir(mTempDir.path()).filePath(QString::fromUtf8("Screenshots"));
        REQUIRE(QDir().mkpath(mWatched));
    }

    // start() records a start time and scanDirectory() ignores anything whose
    // birth time predates it, so a file written in the same millisecond as
    // start() can be treated as pre-existing. Step clear of that boundary
    // rather than racing it: the filesystem birth time and
    // QDateTime::currentDateTime() have been observed to disagree by ~1ms.
    static void settleClockAfterStart()
    {
        QThread::msleep(50);
    }

    // Lets the real QFileSystemWatcher deliver what it saw, for the cases where
    // what it does (or fails to) report is the point of the test.
    static void processWatcherEvents()
    {
        QEventLoop loop;
        QTimer::singleShot(300, &loop, &QEventLoop::quit);
        loop.exec();
    }

    QString write(const QString& name, int bytes) const
    {
        const QString path = QDir(mWatched).filePath(name);
        QFile file(path);
        REQUIRE(file.open(QIODevice::WriteOnly));
        if (bytes > 0)
        {
            REQUIRE(file.write(QByteArray(bytes, 'x')) == bytes);
        }
        file.close();
        return QDir::toNativeSeparators(QFileInfo(path).absoluteFilePath());
    }

    QTemporaryDir mTempDir;
    QString mWatched;
};

} // namespace

TEST_CASE_METHOD(StabilityFixture, "stability tick reports a settled capture exactly once")
{
    TestableDetector detector(mWatched);
    CaptureRecorder recorder(detector);

    detector.start();
    REQUIRE(detector.isRunning());
    settleClockAfterStart();

    const QString capture = write(QString::fromUtf8("Screenshot_1.png"), 512);
    detector.rescan(mWatched);

    // Two ticks at an unchanged size settle it; the extra ticks must not
    // report it again.
    detector.tick(6);

    REQUIRE(recorder.count() == 1);
    CHECK(recorder.first() == capture);
}

TEST_CASE_METHOD(StabilityFixture, "a nested tick cannot report the same capture twice")
{
    TestableDetector detector(mWatched);
    CaptureRecorder recorder(detector);

    // Re-enter the tick from inside the signal, the way QmlDialogWrapper and
    // EventUpdater do, while the outer pass is still walking its snapshot of
    // pending files.
    recorder.setOnCapture(
        [&detector]()
        {
            QEventLoop nested;
            QTimer::singleShot(0,
                               [&nested, &detector]()
                               {
                                   detector.tick();
                                   nested.quit();
                               });
            nested.exec();
        });

    detector.start();
    REQUIRE(detector.isRunning());
    settleClockAfterStart();

    // Two captures pending at once is what makes the re-entrancy observable:
    // the nested pass can settle and remove the second file while the outer
    // pass still holds its key.
    const QString first = write(QString::fromUtf8("Screenshot_A.png"), 256);
    const QString second = write(QString::fromUtf8("Screenshot_B.png"), 256);
    detector.rescan(mWatched);

    detector.tick(8);

    CHECK(recorder.countOf(first) == 1);
    CHECK(recorder.countOf(second) == 1);
    CHECK(recorder.count() == 2);
}

TEST_CASE_METHOD(StabilityFixture, "a file stuck at zero bytes is abandoned, not polled forever")
{
    TestableDetector detector(mWatched);
    CaptureRecorder recorder(detector);

    detector.start();
    REQUIRE(detector.isRunning());
    settleClockAfterStart();

    // A capture tool that created the file and died before writing it. Zero
    // bytes satisfies neither the "settled" nor the "changed" branch, so the
    // entry used to stay pending and keep the 150ms timer alive all session.
    const QString stuck = write(QString::fromUtf8("Screenshot_empty.png"), 0);
    detector.rescan(mWatched);

    detector.tick(MORE_THAN_MAX_PENDING_TICKS);
    CHECK(recorder.countOf(stuck) == 0);

    // Positive control. Without it a broken detector that reports nothing at all
    // would satisfy every other assertion in this test.
    const QString healthy = write(QString::fromUtf8("Screenshot_ok.png"), 256);
    detector.rescan(mWatched);
    detector.tick(6);
    REQUIRE(recorder.countOf(healthy) == 1);

    // Distinguishes "abandoned" from "still pending": the stuck file was
    // recorded as known, so even once it gains content it is not reported. Had
    // it merely been left pending, this would settle and fire.
    QFile file(stuck);
    REQUIRE(file.open(QIODevice::WriteOnly));
    file.write(QByteArray(128, 'x'));
    file.close();

    detector.rescan(mWatched);
    detector.tick(6);
    CHECK(recorder.countOf(stuck) == 0);
}

TEST_CASE_METHOD(StabilityFixture, "a capture removed before settling is dropped")
{
    TestableDetector detector(mWatched);
    CaptureRecorder recorder(detector);

    detector.start();
    REQUIRE(detector.isRunning());
    settleClockAfterStart();

    const QString transient = write(QString::fromUtf8("Screenshot_gone.png"), 512);
    detector.rescan(mWatched);

    // Detected, then deleted before its size settled -- what happens when a
    // capture is moved or removed inside the settle window.
    detector.tick();
    REQUIRE(QFile::remove(transient));
    detector.tick(6);

    CHECK(recorder.countOf(transient) == 0);

    // Positive control, for the same reason as above: prove the detector is
    // still reporting, so the zero is about the removed file specifically.
    const QString healthy = write(QString::fromUtf8("Screenshot_ok.png"), 256);
    detector.rescan(mWatched);
    detector.tick(6);
    REQUIRE(recorder.countOf(healthy) == 1);
}

TEST_CASE_METHOD(StabilityFixture, "files present before start() are baselined, never uploaded")
{
    // Everything already in the folder when the detector starts belongs to the
    // user's past, not to this session. Reporting it would bulk-upload (and,
    // with link copying on, publicly export) the folder's history.
    const QString preexisting = write(QString::fromUtf8("Screenshot_old.png"), 512);

    TestableDetector detector(mWatched);
    CaptureRecorder recorder(detector);
    detector.start();
    settleClockAfterStart();

    detector.rescan(mWatched);
    detector.tick(6);
    CHECK(recorder.countOf(preexisting) == 0);

    // A capture taken after start is what the detector exists for.
    const QString fresh = write(QString::fromUtf8("Screenshot_new.png"), 512);
    detector.rescan(mWatched);
    detector.tick(6);
    REQUIRE(recorder.countOf(fresh) == 1);
    CHECK(recorder.count() == 1);
}

TEST_CASE_METHOD(StabilityFixture, "an old image copied into the folder is not a capture")
{
    TestableDetector detector(mWatched);
    CaptureRecorder recorder(detector);
    detector.start();
    settleClockAfterStart();

    // Explorer, Finder and Nautilus give a copy a new birth time but keep the
    // source's modification time. A photo library filed under "Screenshots"
    // looks exactly like this, and must not be uploaded (nor publicly
    // exported, with link copying on).
    const QString copiedName = QString::fromUtf8("IMG_0042.png");
    const QString copied = write(copiedName, 512);
    {
        QFile file(QDir(mWatched).filePath(copiedName));
        REQUIRE(file.open(QIODevice::ReadWrite));
        REQUIRE(file.setFileTime(QDateTime::currentDateTime().addDays(-30),
                                 QFileDevice::FileModificationTime));
        file.close();
    }
    detector.rescan(mWatched);
    detector.tick(6);
    CHECK(recorder.countOf(copied) == 0);

    // A real capture is written now on both counts.
    const QString fresh = write(QString::fromUtf8("Screenshot_new.png"), 512);
    detector.rescan(mWatched);
    detector.tick(6);
    REQUIRE(recorder.countOf(fresh) == 1);
    CHECK(recorder.count() == 1);
}

TEST_CASE_METHOD(StabilityFixture,
                 "an old image is not a capture when its timestamps arrive after its data")
{
    TestableDetector detector(mWatched);
    CaptureRecorder recorder(detector);
    detector.start();
    settleClockAfterStart();

    // Copy tools write the data first and set the source's timestamps last,
    // so the scan triggered by the file's appearance still sees fresh ones;
    // the old modification time is only there once the copy has finished. A
    // photo of a few megabytes is routinely still being copied at that scan.
    const QString copiedName = QString::fromUtf8("IMG_0043.png");
    const QString copied = write(copiedName, 512);
    detector.rescan(mWatched); // seen while the copy is still in progress
    {
        QFile file(QDir(mWatched).filePath(copiedName));
        REQUIRE(file.open(QIODevice::ReadWrite));
        REQUIRE(file.setFileTime(QDateTime::currentDateTime().addDays(-30),
                                 QFileDevice::FileModificationTime));
        file.close();
    }
    detector.tick(6);
    CHECK(recorder.countOf(copied) == 0);

    // A real capture written now is still reported.
    const QString fresh = write(QString::fromUtf8("Screenshot_new.png"), 512);
    detector.rescan(mWatched);
    detector.tick(6);
    REQUIRE(recorder.countOf(fresh) == 1);
    CHECK(recorder.count() == 1);
}

TEST_CASE_METHOD(StabilityFixture, "a file still being written is not reported until it settles")
{
    TestableDetector detector(mWatched);
    CaptureRecorder recorder(detector);
    detector.start();
    settleClockAfterStart();

    // A capture tool streaming the PNG out: the size keeps moving.
    const QString growing = write(QString::fromUtf8("Screenshot_grow.png"), 100);
    detector.rescan(mWatched);
    detector.tick(2);
    CHECK(recorder.count() == 0); // one stable tick is not enough

    QFile file(QDir(mWatched).filePath(QString::fromUtf8("Screenshot_grow.png")));
    REQUIRE(file.open(QIODevice::Append));
    file.write(QByteArray(200, 'y'));
    file.close();

    // The growth resets the stability count; only after the size holds for two
    // consecutive ticks is the capture reported -- and exactly once.
    detector.tick();
    CHECK(recorder.count() == 0);
    detector.tick(6);
    REQUIRE(recorder.countOf(growing) == 1);
    CHECK(recorder.count() == 1);
}

TEST_CASE_METHOD(StabilityFixture, "macOS dot-file temporaries are ignored until renamed")
{
    // macOS writes ".Screenshot ….png" first and renames it to the visible
    // name when the capture is complete. The dot-file must not be reported;
    // the renamed file must.
    TestableDetector detector(mWatched);
    CaptureRecorder recorder(detector);
    detector.start();
    settleClockAfterStart();

    const QString hiddenName = QString::fromUtf8(".Screenshot 2026-06-10.png");
    write(hiddenName, 512);
    detector.rescan(mWatched);
    detector.tick(6);
    CHECK(recorder.count() == 0);

    const QString finalPath =
        QDir(mWatched).filePath(QString::fromUtf8("Screenshot 2026-06-10.png"));
    REQUIRE(QFile::rename(QDir(mWatched).filePath(hiddenName), finalPath));
    detector.rescan(mWatched);
    detector.tick(6);
    REQUIRE(recorder.countOf(QDir::toNativeSeparators(finalPath)) == 1);
    CHECK(recorder.count() == 1);
}

TEST_CASE_METHOD(StabilityFixture, "only image extensions count, even in a dedicated folder")
{
    // A dedicated capture folder accepts any *image*, whatever its name -- but
    // never other file types. Screen recordings land next to screenshots on
    // every OS and must not be uploaded by this feature.
    TestableDetector detector(mWatched);
    CaptureRecorder recorder(detector);
    detector.start();
    settleClockAfterStart();

    write(QString::fromUtf8("Screen Recording 2026-06-10.mov"), 512);
    write(QString::fromUtf8("Screenshot notes.txt"), 512);
    write(QString::fromUtf8("Screenshot animation.gif"), 512);
    detector.rescan(mWatched);
    detector.tick(6);
    CHECK(recorder.count() == 0);

    // Dedicated folder: an image with a non-capture name still counts, and the
    // extension check is case-insensitive.
    const QString localized = write(QString::fromUtf8("cualquier imagen.jpeg"), 512);
    const QString upper = write(QString::fromUtf8("Screenshot_upper.PNG"), 512);
    detector.rescan(mWatched);
    detector.tick(6);
    CHECK(recorder.countOf(localized) == 1);
    CHECK(recorder.countOf(upper) == 1);
    CHECK(recorder.count() == 2);
}

TEST_CASE_METHOD(StabilityFixture, "shared folders apply the name filter end to end")
{
    // The statics have their own tests; this exercises the scanDirectory path
    // that combines them: in a folder not named "Screenshots" a new image is
    // only a capture if its NAME says so.
    const QString shared = QDir(mTempDir.path()).filePath(QString::fromUtf8("Pictures"));
    REQUIRE(QDir().mkpath(shared));

    TestableDetector detector(shared);
    CaptureRecorder recorder(detector);
    detector.start();
    settleClockAfterStart();

    const auto writeShared = [&shared](const QString& name, int bytes)
    {
        const QString path = QDir(shared).filePath(name);
        QFile file(path);
        REQUIRE(file.open(QIODevice::WriteOnly));
        REQUIRE(file.write(QByteArray(bytes, 'x')) == bytes);
        file.close();
        return QDir::toNativeSeparators(QFileInfo(path).absoluteFilePath());
    };

    const QString photo = writeShared(QString::fromUtf8("IMG_20260610_140533.png"), 512);
    detector.rescan(shared);
    detector.tick(6);
    CHECK(recorder.countOf(photo) == 0);

    const QString capture =
        writeShared(QString::fromUtf8("Captura de pantalla 2026-06-10.png"), 512);
    detector.rescan(shared);
    detector.tick(6);
    REQUIRE(recorder.countOf(capture) == 1);
    CHECK(recorder.count() == 1);
}

TEST_CASE_METHOD(StabilityFixture, "stop() and start() rebuild the baseline")
{
    TestableDetector detector(mWatched);
    CaptureRecorder recorder(detector);
    detector.start();
    settleClockAfterStart();

    const QString sessionOne = write(QString::fromUtf8("Screenshot_s1.png"), 512);
    detector.rescan(mWatched);
    detector.tick(6);
    REQUIRE(recorder.countOf(sessionOne) == 1);

    // The feature was toggled off (or the user logged out); anything saved in
    // the meantime was not captured under the detector's watch.
    detector.stop();
    REQUIRE_FALSE(detector.isRunning());
    const QString whileStopped = write(QString::fromUtf8("Screenshot_off.png"), 512);

    detector.start();
    settleClockAfterStart();
    detector.rescan(mWatched);
    detector.tick(6);
    CHECK(recorder.countOf(whileStopped) == 0);
    CHECK(recorder.countOf(sessionOne) == 1); // not re-reported either

    const QString sessionTwo = write(QString::fromUtf8("Screenshot_s2.png"), 512);
    detector.rescan(mWatched);
    detector.tick(6);
    REQUIRE(recorder.countOf(sessionTwo) == 1);
}

TEST_CASE_METHOD(StabilityFixture, "a rescan of pending files does not duplicate the report")
{
    // The watcher fires directoryChanged several times while a capture is being
    // written; every one triggers a scan of the same not-yet-settled file.
    TestableDetector detector(mWatched);
    CaptureRecorder recorder(detector);
    detector.start();
    settleClockAfterStart();

    const QString capture = write(QString::fromUtf8("Screenshot_multi.png"), 512);
    detector.rescan(mWatched);
    detector.rescan(mWatched);
    detector.tick();
    detector.rescan(mWatched);
    detector.tick(6);
    detector.rescan(mWatched); // and after it was reported
    detector.tick(6);

    REQUIRE(recorder.countOf(capture) == 1);
    CHECK(recorder.count() == 1);
}

TEST_CASE_METHOD(StabilityFixture, "a capture folder deleted and recreated is picked up again")
{
    // The OS (or the user) removes the capture folder mid-session and it comes
    // back later together with a first capture. QFileSystemWatcher forgets
    // deleted paths silently, so the detector demotes the folder, watches its
    // parent, and must re-attach when it reappears -- without swallowing that
    // first capture as "pre-existing".
    TestableDetector detector(mWatched);
    CaptureRecorder recorder(detector);
    detector.start();
    settleClockAfterStart();

    REQUIRE(QDir(mWatched).removeRecursively());
    detector.rescan(mWatched); // watcher reports the change; folder is gone

    REQUIRE(QDir().mkpath(mWatched));
    const QString firstCapture = write(QString::fromUtf8("Screenshot_back.png"), 512);
    detector.rescan(mTempDir.path()); // the watched parent sees the folder return

    detector.tick(6);
    REQUIRE(recorder.countOf(firstCapture) == 1);
    CHECK(recorder.count() == 1);
}

TEST_CASE_METHOD(StabilityFixture,
                 "a capture folder moved to the trash and recreated is picked up again")
{
    // Deleting from a file manager moves the folder to the trash. Unlike a real
    // delete, QFileSystemWatcher does not report that rename for the folder
    // itself and keeps it listed as watched, so only the parent sees it go.
    // That is the behaviour under test, so this drives the real watcher rather
    // than rescan().
    TestableDetector detector(mWatched);
    CaptureRecorder recorder(detector);
    detector.start();
    settleClockAfterStart();

    const QString trash = QDir(mTempDir.path()).filePath(QString::fromUtf8("Trash"));
    REQUIRE(QDir().mkpath(trash));
    REQUIRE(QDir().rename(mWatched, QDir(trash).filePath(QString::fromUtf8("Screenshots"))));
    processWatcherEvents();

    REQUIRE(QDir().mkpath(mWatched));
    processWatcherEvents();

    // Written only once the folder is back, so only a watch on the recreated
    // folder can see it.
    const QString firstCapture = write(QString::fromUtf8("Screenshot_back.png"), 512);
    processWatcherEvents();

    detector.tick(6);
    REQUIRE(recorder.countOf(firstCapture) == 1);
    CHECK(recorder.count() == 1);
}
