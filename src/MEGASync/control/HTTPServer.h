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

class RequestData
{
public:
    enum
    {
        STATE_OK   = 0,          ///< Everything OK
        STATE_OPEN = -201,       ///< Selection dialog is still open.
        STATE_CANCELLED = -202,  ///< Selection dialog cancelled by user.
    };

    RequestData() : files(-1), folders(-1), ts(QDateTime::currentMSecsSinceEpoch()), status(STATE_OPEN) {}
    int files;
    int folders;
    long long ts;
    int status;
};

class RequestTransferData
{
public:
    enum
    {
        STATE_NONE = 0,
        STATE_QUEUED = 1,
        STATE_ACTIVE = 2,
        STATE_PAUSED = 3,
        STATE_RETRYING = 4,
        STATE_COMPLETING = 5,
        STATE_COMPLETED = 6,
        STATE_CANCELLED = 7,
        STATE_FAILED = 8
    };

    RequestTransferData()
        : state(STATE_NONE), progress(0), size(0), speed(0) {}
    int state;
    long long progress;
    long long size;
    long long speed;
};

class HTTPRequest
{
public:
    HTTPRequest() : contentLength(0), origin(-1) {}
    QString data;
    int contentLength;
    int origin;
};

class HTTPServer: public QTcpServer
{
    Q_OBJECT

    public:
        HTTPServer(mega::MegaApi *megaApi, quint16 port, bool sslEnabled);
        ~HTTPServer();
#if QT_VERSION >= 0x050000
        void incomingConnection(qintptr socket);
#else
        void incomingConnection(int socket);
#endif
        void pause();
        void resume();

        void onUploadSelectionAccepted(int files, int folders);
        void onUploadSelectionDiscarded();

        void onTransferDataUpdate(mega::MegaHandle handle, int state, long long progress, long long size, long long speed);

    signals:
        void onLinkReceived(QString link, QString auth);
        void onExternalDownloadRequested(QQueue<mega::MegaNode*> files);
        void onExternalDownloadRequestFinished();
        void onExternalFileUploadRequested(qlonglong targetHandle);
        void onExternalFolderUploadRequested(qlonglong targetHandle);
        void onExternalFolderSyncRequested(qlonglong targetHandle);
        void onExternalOpenTransferManagerRequested(int tab);

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
        bool isFirstWebDownloadDone;
        mega::MegaApi *megaApi;
        QMap<QAbstractSocket*, HTTPRequest*> requests;
        QMultiMap<QString, RequestData*> webDataRequests;
        QMap<mega::MegaHandle, RequestTransferData*> webTransferStateRequests;
};

#endif // HTTPSERVER_H
