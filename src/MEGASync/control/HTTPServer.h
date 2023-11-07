#ifndef HTTPSERVER_H
#define HTTPSERVER_H

#include <QTcpServer>
#include <QSslSocket>
#include <QSslKey>
#include <QFile>
#include <QStringList>
#include <QDateTime>
#include <QQueue>
#include <QFutureWatcher>
#include <QPointer>

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

    enum RequestType
    {
        VERSION_COMMAND = 0,
        OPEN_LINK_REQUEST_START,
        EXTERNAL_DOWNLOAD_REQUEST_START,
        EXTERNAL_FILE_UPLOAD_REQUEST_START,
        EXTERNAL_FOLDER_UPLOAD_REQUEST_START,
        EXTERNAL_FOLDER_SYNC_REQUEST_START,
        EXTERNAL_FOLDER_SYNC_CHECK_START,
        EXTERNAL_OPEN_TRANSFER_MANAGER_START,
        EXTERNAL_UPLOAD_SELECTION_STATUS_START,
        EXTERNAL_TRANSFER_QUERY_PROGRESS_START,
        EXTERNAL_SHOW_IN_FOLDER,
        EXTERNAL_ADD_BACKUP,
        UNKNOWN_REQUEST,
        EXTERNAL_REWIND_REQUEST_START,
    };

    public:
        static const unsigned int MAX_REQUEST_TIME_SECS;

        HTTPServer(mega::MegaApi *megaApi, quint16 port);
        ~HTTPServer();

        void incomingConnection(qintptr socket);
        void pause();
        void resume();

        static void checkAndPurgeRequests();
        static void onUploadSelectionAccepted(int files, int folders);
        static void onUploadSelectionDiscarded();
        static void onTransferDataUpdate(mega::MegaHandle handle, int state, long long progress,
                                         long long size, long long speed, QString localPath);

    signals:
        void onLinkReceived(QString link, QString auth);
        void onExternalDownloadRequested(QQueue<WrappedNode *> files);
        void onExternalDownloadRequestFinished();
        void onExternalFileUploadRequested(qulonglong targetHandle);
        void onExternalFolderUploadRequested(qulonglong targetHandle);
        void onExternalFolderSyncRequested(qulonglong targetHandle);
        void onExternalOpenTransferManagerRequested(int tab);
        void onExternalShowInFolderRequested(QString path);
        void onExternalAddBackup();

    private slots:
        void onVersionCommandFinished();

    public slots:
        void readClient();
        void discardClient();
        void rejectRequest(QAbstractSocket *socket, QString response = QString::fromUtf8("403 Forbidden"));
        void processRequest(QPointer<QAbstractSocket> socket, HTTPRequest request);

    private:
        QString findCorrespondingAllowedOrigin(const QStringList& headers);

        void processPostRequest(QAbstractSocket* socket, HTTPRequest* request,
                                const QStringList& headers, const QString& content);
        void processOptionRequest(QAbstractSocket* socket, HTTPRequest* request, const QStringList& headers);
        void sendPreFlightResponse(QAbstractSocket* socket, HTTPRequest* request, bool sendPrivateNetworkField);
        bool hasFieldWithValue(const QStringList& headers, const char* fieldName, const char* value);
        bool isPreFlightCorsRequest(const QStringList& headers);
        bool isRequestOfType(const QStringList& headers, const char* typeName);

        struct VersionCommandAnswer
        {
            QPointer<QAbstractSocket> socket;
            HTTPRequest request;
            QString response;
        };

        void versionCommand(const HTTPRequest &request, QPointer<QAbstractSocket> socket);
        void openLinkRequest(QString& response, const HTTPRequest& request);
        void externalDownloadRequest(QString& response, const HTTPRequest& request, QAbstractSocket* socket, bool undelete = false);
        void externalFileUploadRequest(QString& response, const HTTPRequest& request);
        void externalFolderUploadRequest(QString& response, const HTTPRequest& request);
        void externalFolderSyncRequest(QString& response, const HTTPRequest& request);
        void externalFolderSyncCheck(QString& response, const HTTPRequest& request);
        void externalOpenTransferManager(QString& response, const HTTPRequest& request);
        void externalUploadSelectionStatus(QString& response, const HTTPRequest& request);
        void externalTransferQueryProgress(QString& response, const HTTPRequest& request);
        void externalShowInFolder(QString& response, const HTTPRequest& request);
        void externalAddBackup(QString& response, const HTTPRequest& request);

        void endProcessRequest(QPointer<QAbstractSocket> socket, const HTTPRequest &request, QString response);

        RequestType GetRequestType(const HTTPRequest& request);
        bool disabled;
        mega::MegaApi *megaApi;
        QMap<QAbstractSocket*, HTTPRequest*> requests;
        static bool isFirstWebDownloadDone;
        static QMultiMap<QString, RequestData*> webDataRequests;
        static QMap<mega::MegaHandle, RequestTransferData*> webTransferStateRequests;
        QFutureWatcher<VersionCommandAnswer> mVersionCommandWatcher;
};

#endif // HTTPSERVER_H
