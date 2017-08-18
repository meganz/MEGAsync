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
        STATE_OPEN = 0,       ///< Selection dialog is still open.
        STATE_OK   = 1,       ///< Everything OK
        STATE_CANCELLED = 2,  ///< Selection dialog cancelled by user.
    };

    RequestData();
    int files;
    int folders;
    long long ts;
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
