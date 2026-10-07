#include "ScreenCaptureDetector.h"

#include "megaapi.h"
#include "Platform.h"

#include <QDir>
#include <QFileInfo>
#include <QRegularExpression>
#include <QScopedValueRollback>

namespace
{
constexpr int STABILITY_INTERVAL_MS = 150;
constexpr int STABLE_TICKS_REQUIRED = 2; // size unchanged for ~300ms => fully written
// ~30s at STABILITY_INTERVAL_MS. Far longer than any real capture takes to
// write, and bounds how long a file that never settles keeps the timer alive.
constexpr int MAX_PENDING_TICKS = 200;
const QStringList IMAGE_EXTENSIONS{QString::fromUtf8("png"),
                                   QString::fromUtf8("jpg"),
                                   QString::fromUtf8("jpeg")};
// Leaf name of the OS-dedicated capture folders the platform layers point at
// (Pictures/Screenshots on Windows and Linux). Keep in sync with
// PlatformImplementation::getScreenshotsLocation().
const QString CAPTURE_FOLDER_NAME{QString::fromUtf8("Screenshots")};
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

    const QStringList directories = captureDirectories();
    for (const QString& directory: directories)
    {
        addCaptureDirectory(directory, true);
    }

    mRunning = !mWatcher.directories().isEmpty();
    logWatchedDirectories();

    if (mRunning)
    {
        // The user can relocate a capture folder at any time; the platform
        // says when (Windows: the Snipping Tool's auto-save folder).
        mLocationChangedConnection = connect(Platform::getInstance(),
                                             &AbstractPlatform::screenshotsLocationChanged,
                                             this,
                                             &ScreenCaptureDetector::onCaptureDirectoriesChanged);
    }
}

void ScreenCaptureDetector::addCaptureDirectory(const QString& directory,
                                                bool baselineExistingFiles)
{
    if (isTooBroadToWatch(directory))
    {
        mega::MegaApi::log(mega::MegaApi::LOG_LEVEL_WARNING,
                           QString::fromUtf8("Screenshot detector: refusing to watch '%1', it is "
                                             "too broad to treat as a capture folder")
                               .arg(directory)
                               .toUtf8()
                               .constData());
        return;
    }

    if (QDir(directory).exists())
    {
        watchCaptureDirectory(directory, baselineExistingFiles);
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

void ScreenCaptureDetector::logWatchedDirectories() const
{
    // The only trace of which folders are covered. A location that resolved
    // to the wrong place is invisible otherwise: nothing fails, captures are
    // just never seen.
    const QStringList watched = mCaptureDirectories.values();
    const QStringList pending = mPendingDirectories.values();
    mega::MegaApi::log(mega::MegaApi::LOG_LEVEL_INFO,
                       QString::fromUtf8("Screenshot detector watching: %1%2")
                           .arg(watched.join(QString::fromUtf8(", ")),
                                pending.isEmpty() ? QString() :
                                                    QString::fromUtf8(" (not created yet: %1)")
                                                        .arg(pending.join(QString::fromUtf8(", "))))
                           .toUtf8()
                           .constData());
}

void ScreenCaptureDetector::onCaptureDirectoriesChanged()
{
    if (!mRunning)
    {
        return;
    }

    const QStringList directories = captureDirectories();
    bool changed = false;
    for (const QString& directory: directories)
    {
        if (mCaptureDirectories.contains(directory) || mPendingDirectories.contains(directory))
        {
            continue;
        }

        // No baseline: a capture taken between the relocation and this call
        // must not be lost. scanDirectory()'s timestamp check keeps anything
        // older than start() out, as for a folder created while running.
        addCaptureDirectory(directory, false);
        if (mCaptureDirectories.contains(directory))
        {
            scanDirectory(directory);
        }
        changed = true;
    }

    // A folder no longer listed is one the user moved the capture location
    // away from. Not while pending: the list is incomplete then, and the
    // folder it lacks may well be the one still in use.
    if (!captureDirectoriesPending())
    {
        QSet<QString> stale = mCaptureDirectories + mPendingDirectories;
        for (const QString& directory: directories)
        {
            stale.remove(directory);
        }
        for (const QString& directory: stale)
        {
            removeCaptureDirectory(directory);
            changed = true;
        }
    }

    if (changed)
    {
        syncWatchedPaths();
        logWatchedDirectories();
    }
}

void ScreenCaptureDetector::removeCaptureDirectory(const QString& directory)
{
    mCaptureDirectories.remove(directory);
    mPendingDirectories.remove(directory);

    // A capture still settling there is dropped with the folder. mKnownFiles
    // is kept: should the user move back, what was already seen stays seen.
    for (auto it = mPendingFiles.begin(); it != mPendingFiles.end();)
    {
        if (QFileInfo(QDir::fromNativeSeparators(it.key())).absolutePath() == directory)
        {
            it = mPendingFiles.erase(it);
        }
        else
        {
            ++it;
        }
    }
}

void ScreenCaptureDetector::syncWatchedPaths()
{
    // What mWatcher must hold: every capture folder, plus the parent of each
    // one not created yet. Anything else is left over from a removed folder.
    QSet<QString> needed = mCaptureDirectories;
    for (const QString& directory: qAsConst(mPendingDirectories))
    {
        needed.insert(QFileInfo(directory).absolutePath());
    }

    const QStringList watched = mWatcher.directories();
    for (const QString& path: watched)
    {
        if (!needed.contains(path))
        {
            mWatcher.removePath(path);
        }
    }
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
    disconnect(mLocationChangedConnection);
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
    // The parent is what reports the folder being moved away (see
    // demoteMissingDirectories), so watch it for the folder's whole lifetime,
    // not only while it is pending.
    watchParentOf(directory);

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

void ScreenCaptureDetector::demoteMissingDirectories()
{
    const auto active = mCaptureDirectories; // copy: mutated while iterating
    for (const QString& directory: active)
    {
        if (QDir(directory).exists())
        {
            continue;
        }

        // Deleting from a file manager moves the folder to the trash, and
        // QFileSystemWatcher (inotify) does not report a watched folder being
        // renamed: it keeps the path listed while following the folder into
        // the trash. Drop that stale watch, or re-adding the path once the OS
        // recreates the folder is refused as "already watched" and no capture
        // is ever seen again. Demote it so it re-attaches when it reappears.
        if (mWatcher.directories().contains(directory))
        {
            mWatcher.removePath(directory);
        }
        mCaptureDirectories.remove(directory);
        mPendingDirectories.insert(directory);
        watchParentOf(directory);
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

QStringList ScreenCaptureDetector::captureDirectories() const
{
    return platformCaptureDirectories();
}

bool ScreenCaptureDetector::captureDirectoriesPending() const
{
    return Platform::getInstance()->isScreenshotsLocationPending();
}

QStringList ScreenCaptureDetector::platformCaptureDirectories()
{
    QStringList directories;

    // Platforms read these from user-controlled settings (the macOS
    // `com.apple.screencapture location` default, the Snipping Tool's registry
    // value), so they can carry trailing separators or mixed separators.
    // Normalize before anything compares them: isSharedDirectory() and
    // mCaptureDirectories both key off the exact string.
    const QStringList platformDirectories = Platform::getInstance()->getScreenshotsLocation();
    for (const QString& directory: platformDirectories)
    {
        if (!directory.isEmpty())
        {
            directories << QDir::cleanPath(directory);
        }
    }
    directories.removeDuplicates();

    return directories;
}

bool ScreenCaptureDetector::looksLikeScreenshot(const QString& fileName)
{
    // Every OS capture tool puts its own word at the START of the file name:
    // "Screenshot 2026-06-10 at 14.05.33.png" (macOS), "Screenshot (12).png"
    // (Windows), "Screenshot from 2026-06-10 14-05-33.png" (GNOME),
    // "Screenshot_20260610_140533.png" (Spectacle), "Captura de pantalla
    // 2026-06-10 a las 14.05.33.png" (localized). So the pattern is anchored
    // and prefix-based.
    //
    // It deliberately no longer accepts an unanchored "captur", nor a bare
    // date-plus-time anywhere in the name. Those also matched ordinary photo,
    // camera-export and scan names ("IMG 2026-06-10 14.05.jpg",
    // "family_capture.jpg"), and in a shared folder such as the Pictures root
    // a match means the file is uploaded and, with link copying enabled,
    // publicly exported. Missing a capture is recoverable; publishing a file
    // the user never captured is not, so this errs towards rejecting.
    //
    // Files landing in a dedicated capture folder skip this check entirely
    // (see isSharedDirectory), so it only gates the shared-folder case.
    //
    // Non-Latin prefixes use PCRE "\x{...}" escapes rather than literal UTF-8
    // bytes: the build does not force a UTF-8 source charset on MSVC, so a
    // non-ASCII string literal would not survive every compiler. Add a locale
    // by adding one alternative.
    //
    // The words come from the OS strings themselves, not from memory: macOS
    // screencaptureui's Localizable.loctable ("Screenshot") and gnome-shell's
    // po files ("Screenshot From %s"). Several languages use a different word
    // on each desktop, so both are listed. "." stands for one accented letter,
    // which is a single code point only in NFC, hence the normalization below.
    //
    // The word is not always the first character: the macOS Arabic name
    // format starts with U+200F RIGHT-TO-LEFT MARK, so that the name lays out
    // right to left. Leading invisible format characters (Unicode Cf) are
    // skipped for that reason; they carry no letters, so skipping them never
    // makes an unrelated name match.
    static const QRegularExpression pattern(
        QString::fromUtf8(
            "^[\\s\\p{Cf}]*("
            "screen[ _-]?shot" // en (older macOS "Screen Shot")
            "|screen[ _-]?capture" // en variants
            "|screenclip" // Windows
            "|capture(?!\\p{L})" // en, fr "Capture d'ecran"; not "captured"
            "|captur[a\\x{103}](?!\\p{L})" // es, pt, ca; ro "Captura de ecran"; not "capturando"
            "|cattura(?!\\p{L})" // it; not "catturato"
            "|schermata(?!\\p{L})" // it GNOME "Schermata del"; not "schermate"
            // it macOS before Sequoia (current macOS writes "Screenshot")
            "|istantanea[ _-]?schermo"
            "|bildschirmfoto" // de
            "|scherm\\x{ad}?afbeelding" // nl macOS, with its soft hyphen
            "|schermafdruk" // nl GNOME
            "|zrzut[ _-]?ekranu" // pl
            "|sn.m(ek|ka)[ _-]?obrazovky" // cs "Snimek", sk "Snimka"
            "|sk.rm(av)?bild" // sv GNOME "Skarmbild", macOS "Skarmavbild"
            "|skjermbilde" // nb
            "|sk.rmbillede" // da
            "|n.ytt.kuva" // fi macOS
            "|kuvakaappaus" // fi GNOME
            "|ekran[ _-]?(g.r.nt.s.|resmi)" // tr GNOME/Windows, macOS
            "|k.perny.(k.p|fot.)" // hu GNOME, macOS
            "|tangkapan[ _-]?(layar|skrin)" // id macOS, ms
            "|cuplikan[ _-]?layar" // id GNOME
            "|slika[ _-]?zaslona" // hr
            "|posnetek[ _-]?zaslona" // sl
            // vi macOS "Anh man hinh" / older "Anh chup Man hinh",
            // GNOME "Chup man hinh"
            "|(.nh( ch.p)?|ch.p)[ _-]?m.n[ _-]?h.nh"
            // ru "Снимок экрана"
            "|\\x{0421}\\x{043d}\\x{0438}\\x{043c}\\x{043e}\\x{043a}"
            "[ _-]?\\x{044d}\\x{043a}\\x{0440}\\x{0430}\\x{043d}\\x{0430}"
            // uk "Знімок екрана"
            "|\\x{0417}\\x{043d}\\x{0456}\\x{043c}\\x{043e}\\x{043a}"
            "[ _-]?\\x{0435}\\x{043a}\\x{0440}\\x{0430}\\x{043d}\\x{0430}"
            // zh: macOS "截屏" (CN), "截圖" (TW), "螢幕截圖" (HK);
            // GNOME "截图", "螢幕快照"; Windows "屏幕截图"
            "|\\x{622a}(\\x{5c4f}|\\x{56fe}|\\x{5716})"
            "|\\x{5c4f}\\x{5e55}\\x{622a}\\x{56fe}"
            "|\\x{87a2}\\x{5e55}(\\x{622a}\\x{5716}|\\x{5feb}\\x{7167})"
            // ja "スクリーンショット"
            "|\\x{30b9}\\x{30af}\\x{30ea}\\x{30fc}\\x{30f3}"
            "\\x{30b7}\\x{30e7}\\x{30c3}\\x{30c8}"
            // ko "스크린샷"
            "|\\x{c2a4}\\x{d06c}\\x{b9b0}\\x{c0f7}"
            // ar macOS "لقطة الشاشة", elsewhere "لقطة شاشة"
            "|\\x{0644}\\x{0642}\\x{0637}\\x{0629}[ _-]?(\\x{0627}\\x{0644})?"
            "\\x{0634}\\x{0627}\\x{0634}\\x{0629}"
            // he "צילום מסך"
            "|\\x{05e6}\\x{05d9}\\x{05dc}\\x{05d5}\\x{05dd}"
            "[ _-]?\\x{05de}\\x{05e1}\\x{05da}"
            // el "Στιγμιότυπο οθόνης"
            "|\\x{03a3}\\x{03c4}\\x{03b9}\\x{03b3}\\x{03bc}\\x{03b9}\\x{03cc}"
            "\\x{03c4}\\x{03c5}\\x{03c0}\\x{03bf}[ _-]?"
            "\\x{03bf}\\x{03b8}\\x{03cc}\\x{03bd}\\x{03b7}\\x{03c2}"
            // hi "स्क्रीनशॉट"
            "|\\x{0938}\\x{094d}\\x{0915}\\x{094d}\\x{0930}\\x{0940}"
            "\\x{0928}\\x{0936}\\x{0949}\\x{091f}"
            // th macOS "ภาพถ่ายหน้าจอ", GNOME "ภาพหน้าจอ"
            "|\\x{0e20}\\x{0e32}\\x{0e1e}(\\x{0e16}\\x{0e48}\\x{0e32}\\x{0e22})?"
            "\\x{0e2b}\\x{0e19}\\x{0e49}\\x{0e32}\\x{0e08}\\x{0e2d}"
            ")"),
        QRegularExpression::CaseInsensitiveOption | QRegularExpression::UseUnicodePropertiesOption);

    // macOS can hand out decomposed names (base letter + combining mark);
    // fold them so an accented letter is one code point, as the pattern
    // assumes. The soft hyphen in the Dutch name is not a combining mark and
    // survives this, which is why the pattern allows it explicitly.
    return pattern.match(fileName.normalized(QString::NormalizationForm_C)).hasMatch();
}

bool ScreenCaptureDetector::isTooBroadToWatch(const QString& directory)
{
    // XDG_PICTURES_DIR="$HOME/" makes PicturesLocation the home directory
    // itself, which would put every new image the user saves anywhere in their
    // home under automatic upload and, with link copying enabled, automatic
    // public export. The name filter is a heuristic, not a permission check,
    // so it is not enough on its own at that breadth. A dedicated capture
    // subfolder underneath is still watched, so captures saved there are
    // picked up as normal.
    const QString cleanDirectory = QDir::cleanPath(directory);
    return cleanDirectory == QDir::cleanPath(QDir::homePath()) || QDir(cleanDirectory).isRoot();
}

bool ScreenCaptureDetector::isSharedDirectory(const QString& path)
{
    // Fail closed: a directory is exempt from the name filter only when it is
    // positively recognized as capture-only. Testing membership in a list of
    // shared roots instead failed open -- any path that did not match a root
    // string exactly was treated as a dedicated capture folder, so every new
    // image in it was uploaded. That included the Desktop whenever the OS
    // reported it un-normalized ("~/Desktop/"), and any relocated capture
    // folder the user had pointed at a directory of their own files.
    return QDir(path).dirName().compare(CAPTURE_FOLDER_NAME, Qt::CaseInsensitive) != 0;
}

void ScreenCaptureDetector::onDirectoryChanged(const QString& path)
{
    // Demote first: a folder moved away and recreated between two events must
    // be re-attached to the new folder, not left on the stale watch.
    demoteMissingDirectories();
    promotePendingDirectories();

    if (!mCaptureDirectories.contains(path))
    {
        // A watched parent changed, or the folder itself was just demoted.
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

        // A capture is written the moment it is taken, so both its birth time
        // and its modification time are recent. An existing image copied into
        // the folder gets a new birth time but keeps its source's modification
        // time (Explorer, Finder and Nautilus all preserve it): that old mtime
        // is what tells it apart from a capture, and keeps a user's photos out
        // of the upload when they file them under "Screenshots". The birth
        // time check still covers a file moved in with a recent mtime.
        //
        // This first look is not final: see onStabilityTimerTimeout.
        if (predatesStart(info))
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

bool ScreenCaptureDetector::predatesStart(const QFileInfo& info) const
{
    const QDateTime birth = info.birthTime();
    return (birth.isValid() && birth < mStartTime) || info.lastModified() < mStartTime;
}

void ScreenCaptureDetector::onStabilityTimerTimeout()
{
    // captureDetected() below is emitted synchronously, and its handler runs
    // arbitrary code: QmlDialogWrapper's constructor spins a QEventLoop while
    // the QML loads, and EventUpdater calls QApplication::processEvents() on
    // the ordinary upload path. Either dispatches this same timer again. A
    // nested tick would settle and remove a file whose key the outer pass is
    // still holding in its snapshot, and the outer pass would then resurrect
    // the entry and report the capture a second time.
    if (mInStabilityTick)
    {
        return;
    }
    const QScopedValueRollback<bool> tickGuard(mInStabilityTick, true);

    const QStringList pending = mPendingFiles.keys();
    for (const QString& filePath: pending)
    {
        // find(), never operator[]: the latter default-inserts a missing key,
        // which is what resurrected an already-reported capture. The entry can
        // legitimately be gone by now -- a re-scan or a nested pass may have
        // settled it while this pass was running.
        auto stateIt = mPendingFiles.find(filePath);
        if (stateIt == mPendingFiles.end())
        {
            continue;
        }

        if (mKnownFiles.contains(filePath))
        {
            // Already reported; never report it twice.
            mPendingFiles.erase(stateIt);
            continue;
        }

        const QFileInfo info(filePath);
        if (!info.exists())
        {
            mPendingFiles.erase(stateIt);
            continue;
        }

        PendingFile& state = *stateIt;
        const qint64 size = info.size();
        if (size > 0 && size == state.lastSize)
        {
            if (++state.stableTicks >= STABLE_TICKS_REQUIRED)
            {
                // Erase and mark known *before* emitting: the handler can spin
                // a nested event loop, and this entry must not be visible to
                // anything that runs inside it.
                mPendingFiles.erase(stateIt);
                mKnownFiles.insert(filePath);

                // Copy tools write the data first and the source's timestamps
                // last, so a copy that was still in progress when the folder
                // was scanned carried fresh timestamps and got this far. Its
                // timestamps are final now that the data has settled, so the
                // decision is taken again on these. That is what keeps a
                // user's old photos, copied in bulk into the capture folder,
                // out of the upload: the small ones are complete by the first
                // scan, the large ones only by now.
                if (predatesStart(info))
                {
                    continue;
                }
                emit captureDetected(filePath);
            }
            continue;
        }

        state.lastSize = size;
        state.stableTicks = 0;

        // Never settled in time: a file stuck at zero bytes would otherwise
        // satisfy neither branch and keep the timer polling forever.
        if (++state.ticksWaited >= MAX_PENDING_TICKS)
        {
            mega::MegaApi::log(mega::MegaApi::LOG_LEVEL_WARNING,
                               QString::fromUtf8("Screenshot detector: '%1' never settled, "
                                                 "giving up on it")
                                   .arg(filePath)
                                   .toUtf8()
                                   .constData());
            mPendingFiles.erase(stateIt);
            mKnownFiles.insert(filePath);
        }
    }

    if (mPendingFiles.isEmpty())
    {
        mStabilityTimer.stop();
    }
}
