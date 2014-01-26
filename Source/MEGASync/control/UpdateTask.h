#ifndef UPDATETASK_H
#define UPDATETASK_H

#include <QCoreApplication>
#include <QApplication>
#include <QProcess>
#include <QThread>
#include <QNetworkAccessManager>
#include <QNetworkRequest>
#include <QNetworkReply>
#include <QStringList>
#include <QTimer>
#include <QDir>
#include <QDirIterator>
#include <QDateTime>

#include "sdk/megaapi.h"

class UpdateTask : public QObject
{
    Q_OBJECT

public:
    explicit UpdateTask(QObject *parent = 0);
    ~UpdateTask();

protected:
   void initialCleanup();
   void finalCleanup();
   void postponeUpdate();
   void downloadFile(QString url);
   QString readNextLine(QNetworkReply *reply);
   bool processUpdateFile(QNetworkReply *reply);
   bool processFile(QNetworkReply *reply);
   bool performUpdate();
   void rollbackUpdate(int fileNum);
   void addToSignature(QString value);
   void addToSignature(QByteArray bytes);
   void initSignature();
   bool checkSignature(QString value);
   bool alreadyInstalled(QString relativePath, QString fileSignature);
   bool alreadyDownloaded(QString relativePath, QString fileSignature);
   bool alreadyExists(QString absolutePath, QString fileSignature);

   QStringList downloadURLs;
   QStringList localPaths;
   QStringList fileSignatures;
   AsymmCipher asymkey;
   QNetworkAccessManager *m_WebCtrl;
   HashSignature *signatureChecker;
   char signature[512];
   int updateVersion;
   int currentFile;
   QDir updateFolder;
   QDir backupFolder;
   QDir appFolder;

   static const unsigned int INITIAL_DELAY_SECS;
   static const unsigned int RETRY_INTERVAL_SECS;
   static const QString UPDATE_CHECK_URL;
   static const QString UPDATE_FOLDER_NAME;
   static const QString BACKUP_FOLDER_NAME;
   static const char PUBLIC_KEY[];

signals:
   void updateCompleted();

private slots:
   void downloadFinished(QNetworkReply* reply);
   void onProxyAuthenticationRequired(const QNetworkProxy&, QAuthenticator*);
   void tryUpdate();

public slots:
   void doWork();

};

#endif // UPDATETASK_H
