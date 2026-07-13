#ifndef UPDATETASK_H
#define UPDATETASK_H

#include "megaapi.h"
#include "Preferences.h"

#include <QApplication>
#include <QCoreApplication>
#include <QDateTime>
#include <QDir>
#include <QDirIterator>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QNetworkRequest>
#include <QProcess>
#include <QStringList>
#include <QThread>
#include <QTimer>

#include <functional>

class UpdateTask : public QObject
{
    Q_OBJECT

public:
    explicit UpdateTask(mega::MegaApi *megaApi, QString appFolder, bool isPublic = false, QObject *parent = 0);
    ~UpdateTask();

    // Receives mega::MegaApi::LOG_LEVEL_* messages from the startup sweep, which runs
    // before the MegaApi logger is available.
    using CleanupLogger = std::function<void(int logLevel, const QString& message)>;

    // Applies the obsolete-file cleanup scheduled by the last applied update (see
    // schedulePendingObsoleteCleanup). Must be called early at application start, before
    // the app bundle symlinks are recreated (the sweep removes any symlink that is not
    // part of the update manifest), and only while holding the single-instance lock, so
    // files are never pulled from under a still-running previous version. The request is
    // only honored when the running binary (runningExecutablePath) is the exact version
    // and installation that scheduled it; an installation refreshed by other means in
    // the meantime, or a request naming another installation, is left untouched.
    static void runPendingObsoleteCleanup(const QString& dataPath,
                                          const QString& runningExecutablePath,
                                          const CleanupLogger& logger);

protected:
   void initialCleanup();
   void finalCleanup();
   void postponeUpdate();
   void downloadFile(const QUrl& url);
   QString readNextLine(QNetworkReply *reply);
   bool processUpdateFile(QNetworkReply *reply);
   bool processFile(QNetworkReply *reply);
   bool performUpdate();
   void rollbackUpdate(int fileNum);
   void schedulePendingObsoleteCleanup();
   static void sweepObsoleteFiles(const QDir& appFolder,
                                  const QDir& backupFolder,
                                  const QStringList& manifestPaths,
                                  const CleanupLogger& logger);
   static void removeEmptyInstallFolders(const QDir& appFolder);
   void addToSignature(QString value);
   void addToSignature(QByteArray bytes);
   void initSignature();
   bool checkSignature(QString value);
   bool alreadyInstalled(QString relativePath, QString fileSignature);
   bool alreadyDownloaded(QString relativePath, QString fileSignature);
   bool alreadyExists(QString absolutePath, QString fileSignature);

   std::shared_ptr<Preferences> preferences;
   QStringList downloadURLs;
   QStringList localPaths;
   QStringList fileSignatures;
   QStringList manifestLocalPaths;
   QNetworkAccessManager *m_WebCtrl;
   mega::MegaHashSignature *signatureChecker;
   char signature[512];
   int updateVersion;
   int currentFile;
   QDir updateFolder;
   QDir backupFolder;
   QDir appFolder;
   QString basePath;
   QTimer *updateTimer;
   QTimer *timeoutTimer;
   bool forceInstall;
   bool running;
   bool forceCheck;
   bool isPublic;
   mega::MegaApi *megaApi;

signals:
   void updateCompleted();
   void updateAvailable(bool requested);
   void updateNotFound(bool requested);
   void installingUpdate(bool requested);
   void updateError();

private slots:
   void downloadFinished(QNetworkReply* reply);
   void onProxyAuthenticationRequired(const QNetworkProxy&, QAuthenticator*);

public slots:
   void startUpdateThread();
   void installUpdate();
   void checkForUpdates();
   void tryUpdate();
   void onTimeout();
};

#endif // UPDATETASK_H
