#include "UpdateTask.h"

#include "Platform.h"
#include "ServiceUrls.h"
#include "Utilities.h"

#include <QAuthenticator>
#include <QDesktopServices>
#include <QSaveFile>
#include <QSet>

#include <algorithm>

using namespace mega;
using namespace std;

namespace
{
const char PENDING_CLEANUP_FILE_NAME[] = "megasync.cleanup";
const int PENDING_CLEANUP_FORMAT_VERSION = 2;

QString normalizedManifestPath(const QString& path)
{
    QString normalized = QDir::cleanPath(QDir::fromNativeSeparators(path));
    while (normalized.startsWith(QLatin1Char('/')))
    {
        normalized.remove(0, 1);
    }

    return normalized == QLatin1String(".") ? QString() : normalized;
}

QString manifestPathKey(const QString& path)
{
    const QString normalized = normalizedManifestPath(path);
#if defined(_WIN32) || defined(__APPLE__)
    return normalized.toCaseFolded();
#else
    return normalized;
#endif
}

// True for entries whose content lives outside the installation: real symbolic links
// everywhere, and NTFS junctions / volume mount points on Windows. The obsolete-file
// sweep must treat them as opaque entries and never descend into them — their targets
// typically hold user data. QFileInfo::isSymLink() is deliberately not used: it is
// false for junctions (QDirIterator descends into them for the same reason) and true
// for .lnk shortcuts, which are plain files that must be swept normally.
bool isLinkEntry(const QFileInfo& info)
{
    return info.isSymbolicLink() || info.isJunction();
}

// Single owner of the backup-folder naming scheme: initialCleanup() prunes these
// folders by prefix, so the creation sites must never drift from the constants.
QString timestampedFolderName(const QString& prefix)
{
    return prefix +
           QDateTime::currentDateTimeUtc().toString(QString::fromLatin1("_dd_MM_yy__hh_mm_ss"));
}
} // namespace

UpdateTask::UpdateTask(MegaApi *megaApi, QString appFolder, bool isPublic, QObject *parent) :
    QObject(parent)
{
    m_WebCtrl = NULL;
    signatureChecker = NULL;
    forceInstall = false;
    running = false;
    forceCheck = false;
    updateTimer = NULL;
    timeoutTimer = NULL;
    this->megaApi = megaApi;
    this->appFolder.setPath(appFolder);
    this->isPublic = isPublic;
}

UpdateTask::~UpdateTask()
{
    delete m_WebCtrl;
    delete signatureChecker;
    delete updateTimer;
    delete timeoutTimer;
}

void UpdateTask::installUpdate()
{
    forceInstall = true;
    tryUpdate();
}

void UpdateTask::checkForUpdates()
{
    forceCheck = true;
    tryUpdate();
}

void UpdateTask::startUpdateThread()
{
    if (m_WebCtrl)
    {
        return;
    }

    updateTimer = new QTimer();
    updateTimer->setSingleShot(false);
    connect(updateTimer, SIGNAL(timeout()), this, SLOT(tryUpdate()));
    timeoutTimer = new QTimer();
    timeoutTimer->setSingleShot(true);
    connect(timeoutTimer, SIGNAL(timeout()), this, SLOT(onTimeout()));

    //Set the working directory
    preferences = Preferences::instance();
    basePath = preferences->getDataPath();

#ifdef __APPLE__
    appFolder.cdUp();
    appFolder.cdUp();
#endif

    updateFolder.setPath(basePath + QDir::separator() + Preferences::UPDATE_FOLDER_NAME);
    m_WebCtrl = new QNetworkAccessManager();
    connect(m_WebCtrl, SIGNAL(finished(QNetworkReply*)), this, SLOT(downloadFinished(QNetworkReply*)));
    connect(m_WebCtrl, SIGNAL(proxyAuthenticationRequired(const QNetworkProxy&, QAuthenticator*)), this, SLOT(onProxyAuthenticationRequired(const QNetworkProxy&, QAuthenticator*)));

    string updatePublicKey = Preferences::UPDATE_PUBLIC_KEY;
    QString updatePubKeyEnv = qEnvironmentVariable("MEGA_UPDATE_PUBLIC_KEY");
    if (!updatePubKeyEnv.isEmpty())
    {
        updatePublicKey = updatePubKeyEnv.toUtf8().constData();
    }

    signatureChecker = new MegaHashSignature(updatePublicKey.c_str());

    updateTimer->start(Preferences::UPDATE_RETRY_INTERVAL_SECS*1000);
    QTimer::singleShot(Preferences::UPDATE_INITIAL_DELAY_SECS*1000, this, SLOT(tryUpdate()));
}

void UpdateTask::tryUpdate()
{
    if (running)
    {
        return;
    }

    MegaApi::log(MegaApi::LOG_LEVEL_INFO, "Starting update check");
    running = true;
    initialCleanup();

    downloadFile(ServiceUrls::getAutoUpdateUrl());
}

void UpdateTask::onTimeout()
{
    timeoutTimer->stop();
    delete m_WebCtrl;
    m_WebCtrl = new QNetworkAccessManager();
    connect(m_WebCtrl, SIGNAL(finished(QNetworkReply*)), this, SLOT(downloadFinished(QNetworkReply*)));
    connect(m_WebCtrl, SIGNAL(proxyAuthenticationRequired(const QNetworkProxy&, QAuthenticator*)), this, SLOT(onProxyAuthenticationRequired(const QNetworkProxy&, QAuthenticator*)));

    postponeUpdate();
}

void UpdateTask::initialCleanup()
{
    //Delete previous backups if possible (they could be still in use)
    //Old location (app folder)
    QStringList subdirs = appFolder.entryList(QDir::Dirs);
    for (int i = 0; i < subdirs.size(); i++)
    {
        if (subdirs[i].startsWith(Preferences::UPDATE_BACKUP_FOLDER_NAME))
        {
            Utilities::removeRecursively(appFolder.absoluteFilePath(subdirs[i]));
        }
    }

    //New location (data folder)
    QDir basePathDir(basePath);
    subdirs = basePathDir.entryList(QDir::Dirs);
    for (int i = 0; i < subdirs.size(); i++)
    {
        if (subdirs[i].startsWith(Preferences::UPDATE_BACKUP_FOLDER_NAME))
        {
            Utilities::removeRecursively(basePathDir.absoluteFilePath(subdirs[i]));
        }
    }

    // The startup obsolete-file sweep backups deliberately escape the purge above
    // (their prefix does not start with "backup"): they are the only recovery path if
    // the sweep ever moves a needed file, so they are kept for a while instead of being
    // deleted on the first update check ~60 s after the sweep created them.
    constexpr int OBSOLETE_BACKUP_RETENTION_DAYS = 30;
    const QDateTime retentionLimit =
        QDateTime::currentDateTimeUtc().addDays(-OBSOLETE_BACKUP_RETENTION_DAYS);
    const QFileInfoList obsoleteBackups = basePathDir.entryInfoList(
        QStringList() << Preferences::OBSOLETE_BACKUP_FOLDER_NAME + QString::fromLatin1("_*"),
        QDir::Dirs | QDir::NoDotAndDotDot);
    for (const QFileInfo& obsoleteBackup: obsoleteBackups)
    {
        if (obsoleteBackup.lastModified().toUTC() <= retentionLimit)
        {
            Utilities::removeRecursively(obsoleteBackup.absoluteFilePath());
        }
    }

    //Remove update folder (old location)
    Utilities::removeRecursively(
                QDir::toNativeSeparators(
                    appFolder.absoluteFilePath(Preferences::UPDATE_FOLDER_NAME)));

    //Initialize update info
    downloadURLs.clear();
    localPaths.clear();
    fileSignatures.clear();
    manifestLocalPaths.clear();
    currentFile = -1;
}

//Called after a successful update
void UpdateTask::finalCleanup()
{
    //Change the version info to skip checking the same update again.
    QApplication::setApplicationVersion(QString::number(updateVersion));

    Platform::getInstance()->updateDisplayVersionAfterAutoUpdate(updateVersion, isPublic);

    //Remove the update folder (new location)
    Utilities::removeRecursively(updateFolder.absolutePath());

#ifdef __APPLE__
    MEGA_SET_PERMISSIONS;
#endif

    emit updateCompleted();
}

void UpdateTask::postponeUpdate()
{
    MegaApi::log(MegaApi::LOG_LEVEL_INFO, "Update task finished. No updates available");

    if (forceInstall)
    {
        emit updateError();
    }
    else
    {
        emit updateNotFound(forceCheck);
    }

    forceInstall = false;
    running = false;
    forceCheck = false;
}

void UpdateTask::downloadFile(const QUrl& url)
{
    MegaApi::log(MegaApi::LOG_LEVEL_DEBUG,
                 QString::fromUtf8("Downloading updated file from %1")
                     .arg(url.toString())
                     .toUtf8()
                     .constData());

    QNetworkRequest request(url);
    request.setAttribute(QNetworkRequest::CacheLoadControlAttribute,
                         QVariant(int(QNetworkRequest::AlwaysNetwork)));
    request.setRawHeader("User-Agent", megaApi->getUserAgent());

    m_WebCtrl->get(request);
    timeoutTimer->start(Preferences::UPDATE_TIMEOUT_SECS*1000);
}

QString UpdateTask::readNextLine(QNetworkReply *reply)
{
    char line[4096];
    int len = static_cast<int>(reply->readLine(line, sizeof(line)));
    if ((len <= 0) || (static_cast<unsigned int>(len - 1) >= sizeof(line)))
    {
        return QString();
    }

    QString qLine(QString::fromUtf8(line));
    return qLine.trimmed();
}

bool UpdateTask::processUpdateFile(QNetworkReply *reply)
{
    MegaApi::log(MegaApi::LOG_LEVEL_DEBUG, "Reading update info");

    QString version = readNextLine(reply);
    if (!version.size())
    {
        MegaApi::log(MegaApi::LOG_LEVEL_WARNING, "Invalid update info");
        return false;
    }

    updateVersion = version.toInt();
    int currentVersion = QApplication::applicationVersion().toInt();
    QString updateCheckVersionEnv = qEnvironmentVariable("MEGA_UPDATE_CHECK_VERSION");
    if (!updateCheckVersionEnv.isEmpty())
    {
        currentVersion = updateCheckVersionEnv.toInt();

        MegaApi::log(MegaApi::LOG_LEVEL_WARNING,"Comparing version against environment variable MEGA_UPDATE_CHECK_VERSION");
    }
    if (updateVersion <= currentVersion)
    {
        MegaApi::log(MegaApi::LOG_LEVEL_INFO,
                     QString::fromUtf8("Update not needed. Current version: %1  Update version: %2")
                     .arg(currentVersion).arg(updateVersion).toUtf8().constData());
        return false;
    }

    MegaApi::log(MegaApi::LOG_LEVEL_INFO, QString::fromUtf8("Update available! Current version: %1  Update version: %2")
                 .arg(currentVersion).arg(updateVersion).toUtf8().constData());

    QString updateSignature = readNextLine(reply);
    if (!updateSignature.size())
    {
        MegaApi::log(MegaApi::LOG_LEVEL_ERROR,"Invalid update info (empty info signature)");
        return false;
    }

    initSignature();
    addToSignature(version);

    while (true)
    {
        QString url = readNextLine(reply);
        if (!url.size())
        {
            break;
        }

        QString localPath = readNextLine(reply);
        if (!localPath.size())
        {
            MegaApi::log(MegaApi::LOG_LEVEL_ERROR,"Invalid update info (empty path)");
            return false;
        }

        QString fileSignature = readNextLine(reply);
        if (!fileSignature.size())
        {
            MegaApi::log(MegaApi::LOG_LEVEL_ERROR,"Invalid update info (empty file signature)");
            return false;
        }

        addToSignature(url);
        addToSignature(localPath);
        addToSignature(fileSignature);

        manifestLocalPaths.append(normalizedManifestPath(localPath));
        if (alreadyInstalled(localPath, fileSignature))
        {
            MegaApi::log(MegaApi::LOG_LEVEL_INFO, QString::fromUtf8("File already installed: %1").arg(localPath).toUtf8().constData());
            continue;
        }

        downloadURLs.append(url);
        localPaths.append(localPath);
        fileSignatures.append(fileSignature);
    }

    // Validated before anything acts on the manifest — including the external-update
    // detection below, which historically ran first: an unauthenticated manifest must
    // neither trigger a reboot nor feed the obsolete-file cleanup keep-list.
    if (!checkSignature(updateSignature))
    {
        MegaApi::log(MegaApi::LOG_LEVEL_ERROR, "Invalid update info (invalid signature)");
        return false;
    }

    if (!downloadURLs.size())
    {
        MegaApi::log(MegaApi::LOG_LEVEL_WARNING, "All files are up to date");

        QDir dataDir(preferences->getDataPath());
        QString appVersionPath = dataDir.filePath(QString::fromLatin1("megasync.version"));
        QFile f(appVersionPath);
        if (f.open(QFile::ReadOnly | QFile::Text))
        {
            QTextStream in(&f);
            const QString versionIn = in.readAll();
            if (versionIn.toInt() > Preferences::VERSION_CODE)
            {
                MegaApi::log(MegaApi::LOG_LEVEL_DEBUG, "External update detected");
                return true;
            }
        }

        // No files to install and no externally applied update: nothing to do.
        // Returning false here avoids running performUpdate() (and its reboot via
        // updateCompleted) on every up-to-date check. Obsolete-file cleanup still
        // runs during real updates, when at least one file differs and is downloaded.
        return false;
    }

    return true;
}

bool UpdateTask::processFile(QNetworkReply *reply)
{
    QByteArray data = reply->readAll();

    //Check signature
    initSignature();
    addToSignature(data);
    if (!checkSignature(fileSignatures[currentFile]))
    {
        MegaApi::log(MegaApi::LOG_LEVEL_ERROR, QString::fromUtf8("Invalid or corrupt file: %1")
                     .arg(updateFolder.absoluteFilePath(localPaths[currentFile])).toUtf8().constData());
        return false;
    }

    //Create the folder for the new file
    QFile localFile(updateFolder.absoluteFilePath(localPaths[currentFile]));
    QFileInfo info(localFile);
    info.absoluteDir().mkpath(QString::fromLatin1("."));

    //Delete the file if it exists.
    localFile.remove();

    //Open the new file
    if (!localFile.open(QIODevice::WriteOnly))
    {
        MegaApi::log(MegaApi::LOG_LEVEL_ERROR, QString::fromUtf8("Error opening local file from writting: %1").arg(info.absoluteFilePath()).toUtf8().constData());
        return false;
    }

    //Write the new file
    qsizetype remainingSize = data.size();
    qsizetype position = 0;
    while (remainingSize > 0)
    {
        const qsizetype written = localFile.write(data.constData() + position, remainingSize);
        if (written == -1)
        {
            MegaApi::log(MegaApi::LOG_LEVEL_ERROR,
                         QString::fromUtf8("Error writting file: %1")
                             .arg(info.absoluteFilePath())
                             .toUtf8()
                             .constData());
            localFile.close();
            return false;
        }
        remainingSize -= written;
        position += written;
    }

    //Save the new file
    if (!localFile.flush())
    {
        MegaApi::log(MegaApi::LOG_LEVEL_ERROR, QString::fromUtf8("Error flushing file: %1").arg(info.absoluteFilePath()).toUtf8().constData());
        localFile.close();
        return false;
    }
    localFile.close();

#ifdef _WIN32
    if (isPublic)
    {
        Platform::getInstance()->makePubliclyReadable(QDir::toNativeSeparators(localFile.fileName()));
    }
#endif

    return true;
}

bool UpdateTask::performUpdate()
{
    MegaApi::log(MegaApi::LOG_LEVEL_INFO, "Applying update...");

    //Create backup folder
    QDir basePathDir(basePath);
    backupFolder.setPath(basePathDir.absoluteFilePath(
        timestampedFolderName(Preferences::UPDATE_BACKUP_FOLDER_NAME)));
    backupFolder.mkdir(QString::fromLatin1("."));

    for (int i = 0; i < localPaths.size(); i++)
    {
        QString file = localPaths[i];

        QFileInfo bakInfo(backupFolder.absoluteFilePath(file));
        QDir bakDir = bakInfo.dir();
        bakDir.mkpath(QString::fromUtf8("."));

        QFileInfo dstInfo(appFolder.absoluteFilePath(file));
        QDir dstDir = dstInfo.dir();
        if (!dstDir.exists())
        {
            dstDir.mkpath(QString::fromUtf8("."));
#ifdef _WIN32
            if (isPublic)
            {
                Platform::getInstance()->makePubliclyReadable(QDir::toNativeSeparators(dstDir.absolutePath()));
            }
#endif
        }

        appFolder.rename(file, backupFolder.absoluteFilePath(file));
        if (!updateFolder.rename(file, appFolder.absoluteFilePath(file)))
        {
            MegaApi::log(MegaApi::LOG_LEVEL_ERROR,
                         QString::fromUtf8("Error installing file: %1 in %2")
                             .arg(file, appFolder.absoluteFilePath(file))
                             .toUtf8()
                             .constData());
            rollbackUpdate(i);
            return false;
        }

        MegaApi::log(MegaApi::LOG_LEVEL_INFO, QString::fromUtf8("File installed: %1").arg(file).toUtf8().constData());
    }

    schedulePendingObsoleteCleanup();

    MegaApi::log(MegaApi::LOG_LEVEL_INFO, "Update successfully installed");
    return true;
}

void UpdateTask::rollbackUpdate(int fileNum)
{
    MegaApi::log(MegaApi::LOG_LEVEL_INFO, "Uninstalling update...");
    for (int i = fileNum; i >= 0; i--)
    {
        QString file = localPaths[i];
        appFolder.rename(file, updateFolder.absoluteFilePath(file));
        backupFolder.rename(file, appFolder.absoluteFilePath(file));
        MegaApi::log(MegaApi::LOG_LEVEL_INFO,QString::fromUtf8("File restored: %1").arg(file).toUtf8().constData());
    }
    MegaApi::log(MegaApi::LOG_LEVEL_INFO, "Update uninstalled");
}

// The obsolete-file sweep is deferred to the next application start (see
// runPendingObsoleteCleanup): sweeping now would pull the assets of the still-running
// previous version from under it (loaded modules can be moved on every platform), and
// the post-update restart can be postponed for a long time while transfers are running.
void UpdateTask::schedulePendingObsoleteCleanup()
{
    if (manifestLocalPaths.isEmpty())
    {
        MegaApi::log(MegaApi::LOG_LEVEL_WARNING,
                     "Skipping obsolete file cleanup because the update manifest is empty");
        return;
    }

    // No files were swapped in by this run: the manifest matched an installation that
    // was refreshed by other means (externally applied update / full installer). The
    // auto-updater did not produce that installation, so its manifest must not be used
    // to sweep it — the version guard at sweep time only covers refreshes that happen
    // AFTER scheduling, not before.
    if (localPaths.isEmpty())
    {
        MegaApi::log(MegaApi::LOG_LEVEL_INFO,
                     "Skipping obsolete file cleanup: update was applied externally");
        return;
    }

    // The sweep validates the install folder against this marker (the manifest-relative
    // path of the application binary). It is recorded here rather than hardcoded at
    // sweep time, so a binary rename cannot silently disable the guard.
    const QString marker =
        normalizedManifestPath(appFolder.relativeFilePath(QCoreApplication::applicationFilePath()));
    const QString markerKey = manifestPathKey(marker);
    const bool markerInManifest = std::any_of(manifestLocalPaths.cbegin(),
                                              manifestLocalPaths.cend(),
                                              [&markerKey](const QString& manifestPath)
                                              {
                                                  return manifestPathKey(manifestPath) == markerKey;
                                              });
    if (marker.isEmpty() || QDir::isAbsolutePath(marker) || !markerInManifest)
    {
        MegaApi::log(MegaApi::LOG_LEVEL_WARNING,
                     QString::fromUtf8("Error scheduling obsolete file cleanup: application "
                                       "binary %1 is not part of the update manifest")
                         .arg(marker)
                         .toUtf8()
                         .constData());
        return;
    }

    // Self-describing request: format version, the update version that scheduled it,
    // the install folder, the application binary marker, then the keep-list. The sweep
    // refuses the request unless it runs inside the exact version that scheduled it, so
    // a keep-list can never be applied to an installation refreshed by other means in
    // the meantime (full installer, emergency updater).
    QByteArray content = QByteArray::number(PENDING_CLEANUP_FORMAT_VERSION);
    content.append('\n');
    content.append(QByteArray::number(updateVersion));
    content.append('\n');
    content.append(appFolder.absolutePath().toUtf8());
    content.append('\n');
    content.append(marker.toUtf8());
    content.append('\n');
    for (const QString& manifestPath: manifestLocalPaths)
    {
        content.append(manifestPath.toUtf8());
        content.append('\n');
    }

    // QSaveFile only renames the finished file into place on commit (and commit() also
    // fails when the file could not be opened), so a crash cannot leave a truncated
    // keep-list behind (which would sweep files the app needs).
    QDir dataDir(preferences->getDataPath());
    QSaveFile pendingFile(dataDir.filePath(QString::fromLatin1(PENDING_CLEANUP_FILE_NAME)));
    if (pendingFile.open(QIODevice::WriteOnly))
    {
        pendingFile.write(content);
    }
    if (!pendingFile.commit())
    {
        MegaApi::log(MegaApi::LOG_LEVEL_WARNING,
                     QString::fromUtf8("Error scheduling obsolete file cleanup: %1")
                         .arg(pendingFile.fileName())
                         .toUtf8()
                         .constData());
        return;
    }

    MegaApi::log(MegaApi::LOG_LEVEL_INFO,
                 "Obsolete file cleanup scheduled for the next application start");
}

void UpdateTask::runPendingObsoleteCleanup(const QString& dataPath,
                                           const QString& runningExecutablePath,
                                           const CleanupLogger& logger)
{
#if defined(_WIN32) || defined(__APPLE__)
    const QString pendingPath =
        QDir(dataPath).filePath(QString::fromLatin1(PENDING_CLEANUP_FILE_NAME));
    QFile pendingFile(pendingPath);
    if (!pendingFile.exists())
    {
        return;
    }

    QStringList lines;
    const bool readOk = pendingFile.open(QFile::ReadOnly | QFile::Text);
    if (readOk)
    {
        lines =
            QString::fromUtf8(pendingFile.readAll()).split(QLatin1Char('\n'), Qt::SkipEmptyParts);
        pendingFile.close();
    }

    // Consume the request up front: the sweep is best effort and must not be retried
    // (and potentially keep failing) on every start; the next update reschedules it.
    // Conversely, never sweep without having consumed the request, so a request that
    // cannot be consumed cannot be applied again on a later start either.
    if (!pendingFile.remove())
    {
        logger(MegaApi::LOG_LEVEL_WARNING,
               QString::fromUtf8("Skipping obsolete file cleanup: cannot remove pending cleanup "
                                 "file %1")
                   .arg(pendingPath));
        return;
    }
    if (!readOk)
    {
        logger(MegaApi::LOG_LEVEL_WARNING,
               QString::fromUtf8("Skipping obsolete file cleanup: cannot read pending cleanup "
                                 "file %1")
                   .arg(pendingPath));
        return;
    }

    // Format: format version, scheduling update version, install folder, application
    // binary marker, then the keep-list (at least one entry).
    if (lines.size() < 5)
    {
        logger(MegaApi::LOG_LEVEL_WARNING,
               QString::fromUtf8("Skipping obsolete file cleanup: invalid pending cleanup file"));
        return;
    }

    bool formatVersionOk = false;
    const int formatVersion = lines.takeFirst().toInt(&formatVersionOk);
    if (!formatVersionOk || formatVersion != PENDING_CLEANUP_FORMAT_VERSION)
    {
        logger(MegaApi::LOG_LEVEL_WARNING,
               QString::fromUtf8("Skipping obsolete file cleanup: unsupported pending cleanup "
                                 "file format"));
        return;
    }

    bool scheduledVersionOk = false;
    const int scheduledVersion = lines.takeFirst().toInt(&scheduledVersionOk);
    if (!scheduledVersionOk || scheduledVersion != Preferences::VERSION_CODE)
    {
        // Not the binary this sweep was scheduled for: the installation was refreshed by
        // other means in the meantime (full installer, emergency updater, downgrade), so
        // the keep-list does not describe it. Discard the request.
        logger(MegaApi::LOG_LEVEL_WARNING,
               QString::fromUtf8("Skipping obsolete file cleanup: scheduled by version %1 but "
                                 "version %2 is running")
                   .arg(scheduledVersion)
                   .arg(Preferences::VERSION_CODE));
        return;
    }

    const QDir appFolder(lines.takeFirst());

    // Safety guard: the folder to sweep is read from a file, so before moving anything
    // out of it make sure it really contains the application binary recorded at schedule
    // time, and that the recorded marker is a sane manifest-relative path that the sweep
    // itself will keep.
    const QString appMarker = normalizedManifestPath(lines.takeFirst());
    const QString markerKey = manifestPathKey(appMarker);
    const bool markerValid = !appMarker.isEmpty() && !QDir::isAbsolutePath(appMarker) &&
                             !appMarker.startsWith(QLatin1String("..")) &&
                             std::any_of(lines.cbegin(),
                                         lines.cend(),
                                         [&markerKey](const QString& manifestPath)
                                         {
                                             return manifestPathKey(manifestPath) == markerKey;
                                         });
    if (!markerValid || !QFile::exists(appFolder.filePath(appMarker)))
    {
        logger(MegaApi::LOG_LEVEL_WARNING,
               QString::fromUtf8("Skipping obsolete file cleanup: unexpected installation path %1")
                   .arg(appFolder.absolutePath()));
        return;
    }

    // The recorded folder must be the installation this process is running from: a
    // stale or corrupted request naming another (same-version) installation must not
    // sweep it. Canonicalization resolves symlinks and relative components on both
    // sides; an empty runningExecutablePath (platform lookup failed) skips the sweep.
    const QString recordedExecutable = QFileInfo(appFolder.filePath(appMarker)).canonicalFilePath();
    const QString runningExecutable = QFileInfo(runningExecutablePath).canonicalFilePath();
    if (recordedExecutable.isEmpty() || runningExecutable.isEmpty() ||
        QString::compare(recordedExecutable, runningExecutable, Qt::CaseInsensitive) != 0)
    {
        logger(MegaApi::LOG_LEVEL_WARNING,
               QString::fromUtf8("Skipping obsolete file cleanup: the request does not target "
                                 "the running installation (%1)")
                   .arg(appFolder.absolutePath()));
        return;
    }

    const QDir backupFolder(QDir(dataPath).absoluteFilePath(
        timestampedFolderName(Preferences::OBSOLETE_BACKUP_FOLDER_NAME)));

    sweepObsoleteFiles(appFolder, backupFolder, lines, logger);
#else
    Q_UNUSED(dataPath)
    Q_UNUSED(runningExecutablePath)
    Q_UNUSED(logger)
#endif
}

// Best effort: a leftover obsolete file is strictly less harmful than failing an already
// fully installed update, so per-file failures are logged and skipped instead of
// aborting. A file that cannot be moved now (e.g. locked by another process) will be
// retried when the next update reschedules the sweep.
void UpdateTask::sweepObsoleteFiles(const QDir& appFolder,
                                    const QDir& backupFolder,
                                    const QStringList& manifestPaths,
                                    const CleanupLogger& logger)
{
    logger(MegaApi::LOG_LEVEL_INFO, QString::fromUtf8("Cleaning obsolete install files..."));

    QSet<QString> expectedPaths;
    for (const QString& manifestPath: manifestPaths)
    {
        expectedPaths.insert(manifestPathKey(manifestPath));
    }

    int removedCount = 0;
    int skippedCount = 0;

    // Explicit walk instead of QDirIterator(Subdirectories): the iterator refuses to
    // descend only into symlinks, so it walks straight through NTFS junctions and volume
    // mount points into content that merely *appears* under the install folder but is
    // user data living elsewhere. Link entries are never pushed, which also makes a
    // self-referencing junction harmless instead of a near-endless recursion.
    QStringList pendingFolders{appFolder.absolutePath()};
    QStringList visitedFolders;
    while (!pendingFolders.isEmpty())
    {
        const QFileInfoList entries = QDir(pendingFolders.takeLast())
                                          .entryInfoList(QDir::AllEntries | QDir::Hidden |
                                                         QDir::System | QDir::NoDotAndDotDot);
        for (const QFileInfo& info: entries)
        {
            const bool linkEntry = isLinkEntry(info);
            if (info.isDir() && !linkEntry)
            {
                pendingFolders.append(info.absoluteFilePath());
                visitedFolders.append(info.absoluteFilePath());
                continue;
            }

            const QString relativePath =
                normalizedManifestPath(appFolder.relativeFilePath(info.absoluteFilePath()));
            if (expectedPaths.contains(manifestPathKey(relativePath)))
            {
                continue;
            }

#ifdef _WIN32
            if (linkEntry)
            {
                // A junction or symlink the user added: its target is their data, so
                // removing the link would destroy their configuration, and moving it
                // into the backup would keep the backup from ever being purged (the
                // SDK's emptydirlocal never deletes reparse points on Windows). On
                // macOS, link entries fall through to the move below on purpose: the
                // app bundle symlinks are never part of the manifest, and the ones the
                // app needs are recreated at startup (processSymLinks).
                logger(MegaApi::LOG_LEVEL_WARNING,
                       QString::fromUtf8("Leaving non-installation link in place: %1")
                           .arg(relativePath));
                skippedCount++;
                continue;
            }
#endif

            const QFileInfo backupInfo(backupFolder.absoluteFilePath(relativePath));
            if (!backupInfo.dir().mkpath(QString::fromUtf8(".")))
            {
                logger(MegaApi::LOG_LEVEL_WARNING,
                       QString::fromUtf8("Error creating obsolete file backup folder: %1")
                           .arg(backupInfo.dir().absolutePath()));
                skippedCount++;
                continue;
            }

            // QFile::rename (unlike QDir::rename) falls back to copy + delete when the
            // source and destination are on different filesystems, which the install and
            // backup folders may well be.
            if (!QFile::rename(info.absoluteFilePath(), backupInfo.absoluteFilePath()))
            {
                logger(MegaApi::LOG_LEVEL_WARNING,
                       QString::fromUtf8("Error moving obsolete file %1 to backup %2")
                           .arg(info.absoluteFilePath(), backupInfo.absoluteFilePath()));
                skippedCount++;
                continue;
            }

            removedCount++;
            logger(MegaApi::LOG_LEVEL_INFO,
                   QString::fromUtf8("Obsolete file removed from install folder: %1")
                       .arg(relativePath));
        }
    }

    // Prune the folders the sweep emptied, children before parents (longer paths first).
    // Only folders the walk above visited qualify — never a link entry: RemoveDirectory
    // on a junction deletes the link even while its target still has content, so a
    // junction must not be offered to rmdir at all.
    std::sort(visitedFolders.begin(),
              visitedFolders.end(),
              [](const QString& lhs, const QString& rhs)
              {
                  return lhs.size() > rhs.size();
              });
    for (const QString& folder: visitedFolders)
    {
        QDir().rmdir(folder);
    }

    logger(
        MegaApi::LOG_LEVEL_INFO,
        QString::fromUtf8("Obsolete install file cleanup completed. Files removed: %1, skipped: %2")
            .arg(removedCount)
            .arg(skippedCount));
}

void UpdateTask::addToSignature(QString value)
{
    QByteArray bytes = value.toLatin1();
    addToSignature(bytes);
}

void UpdateTask::addToSignature(QByteArray bytes)
{
    signatureChecker->add(bytes.constData(), static_cast<unsigned>(bytes.length()));
}

void UpdateTask::initSignature()
{
    signatureChecker->init();
}

bool UpdateTask::checkSignature(QString value)
{
    int result = signatureChecker->checkSignature(value.toLatin1().constData());
    if (!result)
    {
        MegaApi::log(MegaApi::LOG_LEVEL_ERROR, "Invalid signature");
    }

    return result;
}

bool UpdateTask::alreadyInstalled(QString relativePath, QString fileSignature)
{
    return alreadyExists(appFolder.absoluteFilePath(relativePath), fileSignature);
}

bool UpdateTask::alreadyDownloaded(QString relativePath, QString fileSignature)
{
    return alreadyExists(updateFolder.absoluteFilePath(relativePath), fileSignature);
}

bool UpdateTask::alreadyExists(QString absolutePath, QString fileSignature)
{
    MegaHashSignature tmpHash((const char *)Preferences::UPDATE_PUBLIC_KEY);
    QFile file(absolutePath);
    if (!file.open(QIODevice::ReadOnly))
    {
        return false;
    }

    QByteArray bytes = file.readAll();
    tmpHash.add(bytes.constData(), static_cast<unsigned>(bytes.size()));
    file.close();

    return tmpHash.checkSignature(fileSignature.toLatin1().constData());
}

void UpdateTask::downloadFinished(QNetworkReply *reply)
{
    timeoutTimer->stop();
    reply->deleteLater();

    //Check if the request has been successful
    QVariant statusCode = reply->attribute( QNetworkRequest::HttpStatusCodeAttribute );
    if (!statusCode.isValid() || (statusCode.toInt() != 200) || (reply->error() != QNetworkReply::NoError))
    {
        MegaApi::log(MegaApi::LOG_LEVEL_ERROR, "Unable to download file");
        postponeUpdate();
        return;
    }

    //Process the received data
    if (currentFile <  0)
    {
        //Process the update file
        if (!processUpdateFile(reply))
        {
            postponeUpdate();
            return;
        }
        emit installingUpdate(forceCheck);
    }
    else
    {
        //Process the file
        if (!processFile(reply))
        {
            MegaApi::log(MegaApi::LOG_LEVEL_ERROR, QString::fromUtf8("Update failed processing file: %1")
                         .arg(downloadURLs[currentFile]).toUtf8().constData());
            postponeUpdate();
            return;
        }
    }

    //File processed. Download the next file
    currentFile++;
    while (currentFile < downloadURLs.size())
    {
        if (!alreadyDownloaded(localPaths[currentFile], fileSignatures[currentFile]))
        {
            MegaApi::log(MegaApi::LOG_LEVEL_INFO, QString::fromLatin1("Downloading file: %1").arg(downloadURLs[currentFile]).toUtf8().constData());
            downloadFile(downloadURLs[currentFile]);
            return;
        }

        MegaApi::log(MegaApi::LOG_LEVEL_INFO, QString::fromLatin1("File already downloaded: %1").arg(localPaths[currentFile]).toUtf8().constData());
        currentFile++;
    }

    //All files have been processed. Apply update
    if (preferences->updateAutomatically() || forceInstall)
    {
        if (!performUpdate())
        {
            postponeUpdate();
            return;
        }

        finalCleanup();
    }
    else
    {
        MegaApi::log(MegaApi::LOG_LEVEL_INFO, "Update installed");
        emit updateAvailable(forceCheck);
        preferences->setLastUpdateTime(QDateTime::currentMSecsSinceEpoch());
        preferences->setLastUpdateVersion(updateVersion);
    }

    forceInstall = false;
    forceCheck = false;
    running = false;
}

void UpdateTask::onProxyAuthenticationRequired(const QNetworkProxy &, QAuthenticator *auth)
{
    auth->setUser(preferences->getProxyUsername());
    auth->setPassword(preferences->getProxyPassword());
}
