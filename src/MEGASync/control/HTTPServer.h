#ifndef HTTPSERVER_H
#define HTTPSERVER_H

#include <QTcpServer>
#include <QSslSocket>
#include <QSslKey>
#include <QFile>
#include <QStringList>
#include <QDateTime>
#include <QQueue>

#include <megaapi.h>

#include "Utilities.h"

class RequestData
{
public:
    enum
    {
        STATE_OPEN = 0,       ///< Selection dialog is still open.
        STATE_OK   = 1,       ///< Everything OK
        STATE_CANCELLED = 2,  ///< Selection dialog cancelled by user.
    };

    RequestData();
    int files;
    int folders;
    long long tsStart;
    long long tsEnd;
    int status;
};

class RequestTransferData
{
public:
    RequestTransferData();
    int state;
    long long progress;
    long long size;
    long long speed;
    long long tsStart;
    long long tsEnd;
    QString tPath;
};

class HTTPRequest
{
public:
    HTTPRequest() : contentLength(0), origin(QString::fromUtf8("*")) {}
    QString data;
    int contentLength;
    QString origin;
};

class HTTPServer: public QTcpServer
{
    Q_OBJECT

    public:
        static const unsigned int MAX_REQUEST_TIME_SECS;

        HTTPServer(mega::MegaApi *megaApi, quint16 port, bool sslEnabled);
        ~HTTPServer();

        void incomingConnection(qintptr socket);
        void pause();
        void resume();

        static void checkAndPurgeRequests();
        static void onUploadSelectionAccepted(int files, int folders);
        static void onUploadSelectionDiscarded();
        static void onTransferDataUpdate(mega::MegaHandle handle, int state, long long progress, long long size, long long speed, QString localPath);

    signals:
        void onLinkReceived(QString link, QString auth);
        void onExternalDownloadRequested(QQueue<WrappedNode *> files);
        void onExternalDownloadRequestFinished();
        void onExternalFileUploadRequested(qlonglong targetHandle);
        void onExternalFolderUploadRequested(qlonglong targetHandle);
        void onExternalFolderSyncRequested(qlonglong targetHandle);
        void onExternalOpenTransferManagerRequested(int tab);
        void onExternalShowInFolderRequested(QString path);
        void onExternalAddBackup();
        void onConnectionError();

    public slots:
        void readClient();
        void discardClient();
        void rejectRequest(QAbstractSocket *socket, QString response = QString::fromUtf8("403 Forbidden"));
        void processRequest(QAbstractSocket *socket, HTTPRequest request);
        void error(QAbstractSocket::SocketError);
        void sslErrors(const QList<QSslError> & errors);
        void peerVerifyError(const QSslError & error);

    private:
        bool disabled;
        bool sslEnabled;
        mega::MegaApi *megaApi;
        QMap<QAbstractSocket*, HTTPRequest*> requests;
        static bool isFirstWebDownloadDone;
        static QMultiMap<QString, RequestData*> webDataRequests;
        static QMap<mega::MegaHandle, RequestTransferData*> webTransferStateRequests;
};

#endif // HTTPSERVER_H
