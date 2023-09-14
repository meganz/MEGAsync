#include "HTTPServer.h"
#include "Preferences/Preferences.h"
#include "AppStatsEvents.h"
#include "Utilities.h"
#include "MegaApplication.h"

#include <QtConcurrent/QtConcurrent>

#include <iostream>


using namespace mega;

const unsigned int HTTPServer::MAX_REQUEST_TIME_SECS = 1800;

bool ts_comparator(RequestData* i, RequestData *j)
{
    return i->tsStart < j->tsStart;
}

RequestData::RequestData()
{
    files = -1;
    folders = -1;
    tsStart = QDateTime::currentMSecsSinceEpoch() / 1000;
    tsEnd = -1;
    status = STATE_OPEN;
}

RequestTransferData::RequestTransferData()
{
    state = MegaTransfer::STATE_NONE;
    progress = 0;
    size = 0;
    speed = 0;
    tsStart = QDateTime::currentMSecsSinceEpoch() / 1000;
    tsEnd = -1;
    tPath = QString();
}

bool HTTPServer::isFirstWebDownloadDone = false;
QMultiMap<QString, RequestData*> HTTPServer::webDataRequests;
QMap<mega::MegaHandle, RequestTransferData*> HTTPServer::webTransferStateRequests;

HTTPServer::HTTPServer(MegaApi *megaApi, quint16 port)
    : QTcpServer(), disabled(false)
{
    this->megaApi = megaApi;
    listen(QHostAddress::LocalHost, port);

    connect(&mVersionCommandWatcher, &QFutureWatcher<VersionCommandAnswer>::finished,
            this, &HTTPServer::onVersionCommandFinished);
}

HTTPServer::~HTTPServer()
{

}

void HTTPServer::incomingConnection(qintptr socket)
{
    if (disabled)
    {
        return;
    }

    MegaApi::log(MegaApi::LOG_LEVEL_DEBUG, "Incoming webclient connection");
    auto preferences = Preferences::instance();
    QTcpSocket* s = new QTcpSocket(this);

    connect(s, SIGNAL(readyRead()), this, SLOT(readClient()));
    connect(s, SIGNAL(disconnected()), this, SLOT(discardClient()));
    connect(s, SIGNAL(error(QAbstractSocket::SocketError)), this, SLOT(error(QAbstractSocket::SocketError)));

    s->setSocketDescriptor(socket);
    requests.insert(s, new HTTPRequest());
}

void HTTPServer::pause()
{
    disabled = true;
}

void HTTPServer::resume()
{
    disabled = false;
}

void HTTPServer::checkAndPurgeRequests()
{
    for (QMultiMap<QString, RequestData*>::iterator it = webDataRequests.begin() ; it != webDataRequests.end();)
    {
        RequestData *requestData = it.value();
        if ((requestData->status == RequestData::STATE_OK || requestData->status == RequestData::STATE_CANCELLED)
                && (((QDateTime::currentMSecsSinceEpoch() / 1000) - requestData->tsEnd) > MAX_REQUEST_TIME_SECS))
        {
            webDataRequests.erase(it++);
            delete requestData;
        }
        else
        {
            it++;
        }
    }

    for (QMap<MegaHandle, RequestTransferData*>::iterator it = webTransferStateRequests.begin() ; it != webTransferStateRequests.end();)
    {
        RequestTransferData *transferData = it.value();
        if ((transferData->state == MegaTransfer::STATE_CANCELLED
             || transferData->state == MegaTransfer::STATE_COMPLETED
             || transferData->state == MegaTransfer::STATE_FAILED)
                && (((QDateTime::currentMSecsSinceEpoch() / 1000) - transferData->tsEnd) > MAX_REQUEST_TIME_SECS))
        {
            webTransferStateRequests.erase(it++);
            delete transferData;
        }
        else
        {
            it++;
        }
    }
}

void HTTPServer::onUploadSelectionAccepted(int files, int folders)
{
    for (QMultiMap<QString, RequestData*>::iterator it = webDataRequests.begin() ; it != webDataRequests.end(); it++)
    {
        if (it.value()->status == RequestData::STATE_OPEN)
        {
            it.value()->status = RequestData::STATE_OK;
            it.value()->files = files;
            it.value()->folders = folders;
            it.value()->tsEnd = QDateTime::currentMSecsSinceEpoch() / 1000;
        }
    }
}

void HTTPServer::onUploadSelectionDiscarded()
{
    for (QMultiMap<QString, RequestData*>::iterator it = webDataRequests.begin() ; it != webDataRequests.end(); it++)
    {
        if (it.value()->status == RequestData::STATE_OPEN)
        {
            it.value()->status = RequestData::STATE_CANCELLED;
            it.value()->tsEnd  = QDateTime::currentMSecsSinceEpoch() / 1000;
        }
    }
}

void HTTPServer::onTransferDataUpdate(MegaHandle handle, int state, long long progress, long long size, long long speed, QString localPath)
{
    QMap<MegaHandle, RequestTransferData*>::iterator it = webTransferStateRequests.find(handle);
    if (it == webTransferStateRequests.end())
    {
        return;
    }

    #ifdef WIN32
    if (localPath.startsWith(QString::fromAscii("\\\\?\\")))
    {
        localPath = localPath.mid(4);
    }
    #endif

    RequestTransferData* tData = it.value();
    tData->state = state;
    tData->progress = progress;
    tData->size = size;
    tData->speed = speed;

    if (!localPath.isEmpty() && tData->tPath != localPath)
    {
        tData->tPath = localPath;
    }

    if (state == MegaTransfer::STATE_CANCELLED
            || state == MegaTransfer::STATE_COMPLETED
            || state == MegaTransfer::STATE_FAILED)
    {
        tData->tsEnd = QDateTime::currentMSecsSinceEpoch() / 1000;
    }
}

void HTTPServer::readClient()
{
    MegaApi::log(MegaApi::LOG_LEVEL_DEBUG, QString::fromUtf8("Processing webclient request via HTTP").toUtf8().constData());
    QAbstractSocket *socket = (QAbstractSocket*)sender();
    HTTPRequest *request = requests.value(socket);
    if (disabled || !request)
    {
        MegaApi::log(MegaApi::LOG_LEVEL_WARNING, "Webclient request not found");
        discardClient();
        return;
    }

    QByteArray socketData = socket->readAll();
    request->data.append(QString::fromUtf8(socketData.data()));
    if (request->data.contains(QString::fromUtf8("\r\n\r\n")))
    {
        QStringList tokens = request->data.split(QString::fromUtf8("\r\n\r\n"));
        QStringList headers = tokens[0].split(QString::fromUtf8("\r\n"));
        bool requestIsPost = isRequestOfType(headers, "POST");
        bool requestIsOption = isRequestOfType(headers, "OPTION");

        if (!headers.size() || (!requestIsPost && !requestIsOption))
        {
            MegaApi::log(MegaApi::LOG_LEVEL_WARNING, "Method not allowed for webclient request");
            rejectRequest(socket, QString::fromUtf8("405 Method Not Allowed"));
            return;
        }

        if (Preferences::HTTPS_ORIGIN_CHECK_ENABLED && !Preferences::HTTPS_ALLOWED_ORIGINS.isEmpty())
        {
            QString foundOrigin = findCorrespondingAllowedOrigin(headers);
            if (!foundOrigin.isEmpty())
            {
                request->origin = foundOrigin;
            }
            else
            {
                MegaApi::log(MegaApi::LOG_LEVEL_DEBUG, "Missing or invalid Origin header");
                rejectRequest(socket);
                return;
            }
        }

        if (requestIsPost)
        {
            processPostRequest(socket, request, headers, tokens[1]);
        }
        else // requestIsOption
        {
            processOptionRequest(socket, request, headers);
        }
    }
}
void HTTPServer::discardClient()
{
    QAbstractSocket* socket = (QSslSocket*)sender();
    socket->deleteLater();

    HTTPRequest *request = requests.value(socket);
    if (request)
    {
        requests.remove(socket);
        delete request;
    }
}

void HTTPServer::rejectRequest(QAbstractSocket *socket, QString response)
{
    socket->write(QString::fromUtf8("HTTP/1.0 %1\r\n"
                  "\r\n").arg(response).toUtf8());
    socket->flush();
    socket->disconnectFromHost();
    socket->deleteLater();

    HTTPRequest *request = requests.value(socket);
    if (request)
    {
        requests.remove(socket);
        delete request;
    }
}

void HTTPServer::processRequest(QPointer<QAbstractSocket> socket, HTTPRequest request)
{
    QString response;

    MegaApi::log(MegaApi::LOG_LEVEL_DEBUG, QString::fromUtf8("Webclient request received: %1").arg(request.data).toUtf8().constData());
    switch(GetRequestType(request))
    {
    case VERSION_COMMAND:
        //Version command is taken using QtConcurrent, this is why the case is broken, as the response is received later
        versionCommand(request, socket);
        return;
    case OPEN_LINK_REQUEST_START:
        openLinkRequest(response, request);
        break;
    case EXTERNAL_DOWNLOAD_REQUEST_START:
        externalDownloadRequest(response, request, socket);
        break;
    case EXTERNAL_FILE_UPLOAD_REQUEST_START:
        externalFileUploadRequest(response, request);
        break;
    case EXTERNAL_FOLDER_UPLOAD_REQUEST_START:
        externalFolderUploadRequest(response, request);
        break;
    case EXTERNAL_FOLDER_SYNC_REQUEST_START:
        externalFolderSyncRequest(response, request);
        break;
    case EXTERNAL_FOLDER_SYNC_CHECK_START:
        externalFolderSyncCheck( response, request);
        break;
    case EXTERNAL_OPEN_TRANSFER_MANAGER_START:
        externalOpenTransferManager(response, request);
        break;
    case EXTERNAL_UPLOAD_SELECTION_STATUS_START:
        externalUploadSelectionStatus(response, request);
        break;
    case EXTERNAL_TRANSFER_QUERY_PROGRESS_START:
        externalTransferQueryProgress(response, request);
        break;
    case EXTERNAL_SHOW_IN_FOLDER:
        externalShowInFolder(response, request);
        break;
    case EXTERNAL_ADD_BACKUP:
        externalAddBackup(response, request);
        break;
    case UNKNOWN_REQUEST:
    default:
        MegaApi::log(MegaApi::LOG_LEVEL_ERROR, QString::fromUtf8("Unknown webclient request: %1").arg(request.data).toUtf8().constData());
        break;
    }

    endProcessRequest(socket,request, response);
}

void HTTPServer::endProcessRequest(QPointer<QAbstractSocket> socket,const HTTPRequest& request, QString response)
{
    if(socket)
    {
        QPointer<HTTPServer> safeServer = this;

        if (!response.size())
        {
            MegaApi::log(MegaApi::LOG_LEVEL_ERROR, QString::fromUtf8("Invalid webclient request: %1").arg(request.data).toUtf8().constData());
            response = QString::number(MegaError::API_EARGS);
        }
        else
        {
            MegaApi::log(MegaApi::LOG_LEVEL_DEBUG, QString::fromUtf8("Response to HTTP request: %1").arg(response).toUtf8().constData());
        }

        QString fullResponse = QString::fromUtf8("HTTP/1.0 200 Ok\r\n"
                                                 "Access-Control-Allow-Origin: %1\r\n"
                                                 "Content-Type: text/html; charset=\"utf-8\"\r\n"
                                                 "Content-Length: %2\r\n"
                                                 "\r\n"
                                                 "%3").arg(request.origin).arg(response.size()).arg(response);
        if (safeServer && socket)
        {
            socket->write(fullResponse.toUtf8());
            socket->flush();
            socket->disconnectFromHost();
            socket->deleteLater();
        }
    }
}

void HTTPServer::versionCommand(const HTTPRequest& request, QPointer<QAbstractSocket> socket)
{
    auto future = QtConcurrent::run([this, socket, request]() -> VersionCommandAnswer
    {
        VersionCommandAnswer answer;

        MegaApi::log(MegaApi::LOG_LEVEL_DEBUG, "GetVersion command received from the webclient");
        char *myHandle = megaApi->getMyUserHandle();
        if (!myHandle)
        {
            answer.response = QString::fromUtf8("{\"v\":\"%1\"}").arg(Preferences::VERSION_STRING);
        }
        else
        {
            answer.response = QString::fromUtf8("{\"v\":\"%1\",\"u\":\"%2\"}")
                                  .arg(Preferences::VERSION_STRING, QString::fromUtf8(myHandle));
            delete [] myHandle;
        }


        answer.request = request;
        answer.socket = socket;

        return answer;
    });

    mVersionCommandWatcher.setFuture(future);
}

void HTTPServer::onVersionCommandFinished()
{
    auto answer = mVersionCommandWatcher.result();

    endProcessRequest(answer.socket, answer.request, answer.response);
}

void HTTPServer::openLinkRequest(QString &response, const HTTPRequest& request)
{
    MegaApi::log(MegaApi::LOG_LEVEL_DEBUG, "OpenLink command received from the webclient");
    QString handle = Utilities::extractJSONString(request.data, QString::fromUtf8("h"));
    QString key = Utilities::extractJSONString(request.data, QString::fromUtf8("k"));
    QString auth = Utilities::extractJSONString(request.data, QString::fromUtf8("esid"));

    if (key.size() > 43)
    {
        key.resize(43);
    }

    if (handle.size() == 8 && key.size() == 43)
    {
        QString link = Preferences::BASE_URL + QString::fromUtf8("/#!%1!%2").arg(handle, key);
        emit onLinkReceived(link, auth);
        response = QString::fromUtf8("0");

        auto preferences = Preferences::instance();
        QString defaultPath = preferences->downloadFolder();
        MegaHandle megaHandle = megaApi->base64ToHandle(handle.toUtf8().constData());
        QMap<MegaHandle, RequestTransferData*>::iterator it = webTransferStateRequests.find(megaHandle);
        if (it != webTransferStateRequests.end())
        {
            delete it.value();
        }
        webTransferStateRequests.insert(megaHandle, new RequestTransferData());

        if (preferences->hasDefaultDownloadFolder() && QFile(defaultPath).exists())
        {
            ((MegaApplication *)qApp)->showInfoMessage(tr("Your download has started"));
        }

        if (!isFirstWebDownloadDone && !preferences->isFirstWebDownloadDone())
        {
            megaApi->sendEvent(AppStatsEvents::EVENT_1ST_WEBCLIENT_DL,
                               "MEGAsync first webclient download", false, nullptr);
            isFirstWebDownloadDone = true;
        }
    }
    else if (key.size() && key.size() != 43)
    {
        response = QString::number(MegaError::API_EKEY);
    }
}

void HTTPServer::externalDownloadRequest(QString &response, const HTTPRequest& request, QAbstractSocket* socket)
{
    QPointer<QAbstractSocket> safeSocket = socket;
    QPointer<HTTPServer> safeServer = this;

    MegaApi::log(MegaApi::LOG_LEVEL_DEBUG, "ExternalDownload command received from the webclient");
    int start = request.data.indexOf(QString::fromUtf8("\"f\":[")) + 5;
    if (start > 0)
    {
        QString privateAuth = Utilities::extractJSONString(request.data.mid(0, start), QString::fromUtf8("esid"));
        QString publicAuth  = Utilities::extractJSONString(request.data.mid(0, start), QString::fromUtf8("en"));
        QString chatAuth    = Utilities::extractJSONString(request.data.mid(0, start), QString::fromUtf8("cauth"));

        if (privateAuth.isEmpty() && publicAuth.isEmpty())
        {
            QString auth  = Utilities::extractJSONString(request.data.mid(0, start), QString::fromUtf8("auth"));
            if (auth.length() == 8)
            {
                publicAuth = auth;
            }
            else
            {
                privateAuth = auth;
            }
        }

        if (privateAuth.size() || publicAuth.size())
        {
            QQueue<WrappedNode *> downloadQueue;

            int end;
            bool firstnode = true;

            while (request.data[start] == QChar::fromAscii('{'))
            {
                end = request.data.indexOf(QChar::fromAscii('}'), start);
                if (end < 0)
                {
                    MegaApi::log(MegaApi::LOG_LEVEL_ERROR, "Error parsing webclient request");
                    qDeleteAll(downloadQueue);
                    downloadQueue.clear();
                    break;
                }

                end++;
                QString file = request.data.mid(start, end - start);
                start = end + 1;

                long long type = Utilities::extractJSONNumber(file, QString::fromUtf8("t"));
                if (type < 0)
                {
                    MegaApi::log(MegaApi::LOG_LEVEL_ERROR, "Node without type in webclient request");
                    qDeleteAll(downloadQueue);
                    downloadQueue.clear();
                    break;
                }

                QString handle = Utilities::extractJSONString(file, QString::fromUtf8("h"));
                if (handle.isEmpty())
                {
                    MegaApi::log(MegaApi::LOG_LEVEL_ERROR, "Node without handle in webclient request");
                    qDeleteAll(downloadQueue);
                    downloadQueue.clear();
                    break;
                }

                QString name = Utilities::extractJSONString(file, QString::fromUtf8("n"));
                name.replace(QString::fromUtf8("-"), QString::fromUtf8("+"));
                name.replace(QString::fromUtf8("_"), QString::fromUtf8("/"));
                name = QString::fromUtf8(QByteArray::fromBase64(name.toUtf8().constData()).constData());
                if (name.isEmpty())
                {
                    MegaApi::log(MegaApi::LOG_LEVEL_ERROR, "Node without name in webclient request");
                    qDeleteAll(downloadQueue);
                    downloadQueue.clear();
                    break;
                }

                MegaHandle h = megaApi->base64ToHandle(handle.toUtf8().constData());
                MegaHandle p = INVALID_HANDLE;

                if (!firstnode)
                {
                    QString parentHandle = Utilities::extractJSONString(file, QString::fromUtf8("p"));
                    p = megaApi->base64ToHandle(parentHandle.toUtf8().constData());
                    QApplication::processEvents();
                    if (!safeServer || !safeSocket)
                    {
                        return;
                    }
                }
                else
                {
                    firstnode = false;
                }

                const QByteArray nameArray = name.toUtf8();
                const QByteArray publicAuthArray = publicAuth.toUtf8();
                const QByteArray privateAuthArray = privateAuth.toUtf8();

                if (type != MegaNode::TYPE_FILE)
                {
                    MegaNode *node = megaApi->createForeignFolderNode(h, nameArray.constData(), p,
                                                                     privateAuthArray.constData(),
                                                                     publicAuthArray.constData());
                    downloadQueue.append(new WrappedNode(WrappedNode::TransferOrigin::FROM_WEBSERVER, node));
                }
                else
                {
                    QString key = Utilities::extractJSONString(file, QString::fromUtf8("k"));
                    if (key.size() == 43)
                    {
                        const QByteArray keyArray = key.toUtf8();
                        long long size = Utilities::extractJSONNumber(file, QString::fromUtf8("s"));
                        long long mtime = Utilities::extractJSONNumber(file, QString::fromUtf8("ts"));

                        QString crc    = Utilities::extractJSONString(file, QString::fromUtf8("c"));
                        const QByteArray crcArray = crc.toUtf8();

                        const QByteArray chatAuthArray = chatAuth.toUtf8();
                        MegaNode *node = megaApi->createForeignFileNode(h, keyArray.constData(),
                                                         nameArray.constData(), size, mtime,
                                                         crc.isEmpty() ? nullptr : crcArray.constData(),
                                                         p, privateAuthArray.constData(),
                                                         publicAuthArray.constData(),
                                                         chatAuth.isEmpty() ? nullptr :  chatAuthArray.constData());
                        downloadQueue.append(new WrappedNode(WrappedNode::TransferOrigin::FROM_WEBSERVER, node));
                        QMap<MegaHandle, RequestTransferData*>::iterator it = webTransferStateRequests.find(h);
                        if (it != webTransferStateRequests.end())
                        {
                            delete it.value();
                        }
                        webTransferStateRequests.insert(h, new RequestTransferData());
                    }
                    else
                    {
                        MegaApi::log(MegaApi::LOG_LEVEL_ERROR, "Node without key (or an invalid key) in webclient request");
                    }
                }
            }

            if (downloadQueue.size())
            {
                emit onExternalDownloadRequested(downloadQueue);
                emit onExternalDownloadRequestFinished();
                response = QString::number(MegaError::API_OK);
            }
        }
    }
}

void HTTPServer::externalFileUploadRequest(QString &response, const HTTPRequest& request)
{
    MegaApi::log(MegaApi::LOG_LEVEL_DEBUG, "UploadFile command received from the webclient");
    QString targetHandle = Utilities::extractJSONString(request.data, QString::fromUtf8("h"));
    MegaHandle handle = ::mega::INVALID_HANDLE;
    if (targetHandle.size())
    {
        handle = MegaApi::base64ToHandle(targetHandle.toUtf8().constData());
    }

    MegaNode *targetNode = megaApi->getNodeByHandle(handle);
    if (!targetNode)
    {
        response = QString::number(MegaError::API_ENOENT);
    }
    else
    {
        delete targetNode;
        QString bid = Utilities::extractJSONString(request.data, QString::fromUtf8("bid"));
        if (!bid.isEmpty())
        {
            webDataRequests.insert(bid, new RequestData());
            emit onExternalFileUploadRequested(handle);
            response = QString::number(MegaError::API_OK);
        }
        else
        {
            response = QString::number(MegaError::API_EARGS);
        }
    }
}

void HTTPServer::externalFolderUploadRequest(QString &response, const HTTPRequest& request)
{
    MegaApi::log(MegaApi::LOG_LEVEL_DEBUG, "UploadFolder command received from the webclient");
    QString targetHandle = Utilities::extractJSONString(request.data, QString::fromUtf8("h"));
    MegaHandle handle = ::mega::INVALID_HANDLE;
    if (targetHandle.size())
    {
        handle = MegaApi::base64ToHandle(targetHandle.toUtf8().constData());
    }

    MegaNode *targetNode = megaApi->getNodeByHandle(handle);
    if (!targetNode)
    {
        response = QString::number(MegaError::API_ENOENT);
    }
    else
    {
        delete targetNode;
        QString bid = Utilities::extractJSONString(request.data, QString::fromUtf8("bid"));
        if (!bid.isEmpty())
        {
            webDataRequests.insert(bid, new RequestData());
            emit onExternalFolderUploadRequested(handle);
            response = QString::number(MegaError::API_OK);
        }
        else
        {
            response = QString::number(MegaError::API_EARGS);
        }
    }
}

void HTTPServer::externalFolderSyncRequest(QString &response, const HTTPRequest& request)
{
    MegaApi::log(MegaApi::LOG_LEVEL_DEBUG, "Sync command received from the webclient");
    QString targetHandle = Utilities::extractJSONString(request.data, QString::fromUtf8("h"));
    MegaHandle handle = ::mega::INVALID_HANDLE;
    if (targetHandle.size())
    {
        handle = MegaApi::base64ToHandle(targetHandle.toUtf8().constData());
    }

    MegaNode *targetNode = megaApi->getNodeByHandle(handle);
    if (!targetNode)
    {
        response = QString::number(MegaError::API_ENOENT);
    }
    else
    {
        delete targetNode;
        emit onExternalFolderSyncRequested(handle);
        response = QString::number(MegaError::API_OK);
    }
}

void HTTPServer::externalFolderSyncCheck(QString &response, const HTTPRequest& request)
{
    MegaApi::log(MegaApi::LOG_LEVEL_DEBUG, "Check sync folder command received from the webclient");
    QString targetHandle = Utilities::extractJSONString(request.data, QString::fromUtf8("h"));
    MegaHandle handle = ::mega::INVALID_HANDLE;
    if (targetHandle.size())
    {
        handle = MegaApi::base64ToHandle(targetHandle.toUtf8().constData());
        MegaNode *targetNode = megaApi->getNodeByHandle(handle);
        if (!targetNode)
        {
            response = QString::number(MegaError::API_ENOENT);
        }
        else
        {
            const char *path = megaApi->getNodePath(targetNode);
            if (path && !strncmp(path, "//bin", 5))
            {
                response = QString::number(MegaError::API_EARGS);
            }
            else
            {
                int result = megaApi->isNodeSyncable(targetNode);
                response = QString::number(result);
            }
            delete [] path;
            delete targetNode;
        }
    }
    else
    {
        response = QString::number(MegaError::API_EARGS);
    }
}

void HTTPServer::externalOpenTransferManager(QString &response, const HTTPRequest& request)
{
    MegaApi::log(MegaApi::LOG_LEVEL_DEBUG, "Open Transfer Manager command received from the webclient");
    int tab = static_cast<int>(Utilities::extractJSONNumber(request.data, QString::fromUtf8("t")));
    if (tab < 0 || tab > 3) //Not valid number tab (all, downloads, uploads, completed)
    {
        response = QString::number(MegaError::API_EARGS);
    }
    else
    {
        emit onExternalOpenTransferManagerRequested(tab);
        response = QString::number(MegaError::API_OK);
    }
}

void HTTPServer::externalUploadSelectionStatus(QString &response, const HTTPRequest& request)
{
    MegaApi::log(MegaApi::LOG_LEVEL_DEBUG, "Upload selection status command received from the webclient");
    QString bid = Utilities::extractJSONString(request.data, QString::fromUtf8("bid"));
    if (!bid.isEmpty())
    {
        QList<RequestData*> values = webDataRequests.values(bid);
        if (!values.isEmpty())
        {
            qSort(values.begin(), values.end(), ts_comparator);
            for (int i = 0; i < values.size(); ++i)
            {
                response.append(i == 0 ? QString::fromUtf8("[") : QString::fromUtf8(","));
                if (values.at(i)->status == RequestData::STATE_OK)
                {
                    response.append(QString::fromUtf8("{\"s\":%1,\"ts\":%2,\"fi\":%3,\"fo\":%4}")
                            .arg(values.at(i)->status)
                            .arg(values.at(i)->tsStart)
                            .arg(values.at(i)->files)
                            .arg(values.at(i)->folders));
                }
                else
                {
                    response.append(QString::fromUtf8("{\"s\":%1,\"ts\":%2}")
                            .arg(values.at(i)->status)
                            .arg(values.at(i)->tsStart));
                }
            }
            response.append(QString::fromUtf8("]"));
        }
        else
        {
            response = QString::number(MegaError::API_ENOENT);
        }
    }
    else
    {
        response = QString::number(MegaError::API_EARGS);
    }
}

void HTTPServer::externalTransferQueryProgress(QString &response, const HTTPRequest& request)
{
    QString targetHandle = Utilities::extractJSONString(request.data, QString::fromUtf8("h"));
    MegaHandle handle = mega::INVALID_HANDLE;
    if (targetHandle.size())
    {
        handle = MegaApi::base64ToHandle(targetHandle.toUtf8().constData());
    }

    if (handle == mega::INVALID_HANDLE)
    {
        response = QString::number(MegaError::API_EARGS);
    }
    else
    {
        if (!webTransferStateRequests.contains(handle))
        {
            response = QString::number(MegaError::API_ENOENT);
        }
        else
        {
            RequestTransferData* tData = webTransferStateRequests.value(handle);
            if (tData->state == MegaTransfer::STATE_NONE)
            {
                response = QString::fromUtf8("{\"s\":%1}").arg(tData->state);
            }
            else
            {
                response = QString::fromUtf8("{\"s\":%1,\"p\":%2,\"t\":%3,\"v\":%4}")
                        .arg(tData->state)
                        .arg(tData->progress)
                        .arg(tData->size)
                        .arg(tData->speed);
            }
        }
    }
}

void HTTPServer::externalShowInFolder(QString &response, const HTTPRequest& request)
{
    MegaApi::log(MegaApi::LOG_LEVEL_DEBUG, "Show in folder command received from the webclient");
    QString targetHandle = Utilities::extractJSONString(request.data, QString::fromUtf8("h"));
    MegaHandle handle = ::mega::INVALID_HANDLE;
    if (targetHandle.size())
    {
        handle = MegaApi::base64ToHandle(targetHandle.toUtf8().constData());
    }

    if (handle == ::mega::INVALID_HANDLE)
    {
        response = QString::number(MegaError::API_EARGS);
    }
    else
    {
        if (!webTransferStateRequests.contains(handle))
        {
            response = QString::number(MegaError::API_ENOENT);
        }
        else
        {
            RequestTransferData* tData = webTransferStateRequests.value(handle);
            if (!tData->tPath.isNull())
            {
                if (QFile(tData->tPath).exists())
                {
                    emit onExternalShowInFolderRequested(tData->tPath);
                }
                else
                {
                    emit onExternalShowInFolderRequested(QFileInfo(tData->tPath).dir().absolutePath());
                }

                response = QString::number(MegaError::API_OK);
            }
            else
            {
                response = QString::number(MegaError::API_ENOENT);
            }
        }
    }
}

void HTTPServer::externalAddBackup(QString &response, const HTTPRequest& request)
{
    MegaApi::log(MegaApi::LOG_LEVEL_DEBUG, "Add backup command received from the webclient");
    QString userHandle(Utilities::extractJSONString(request.data, QLatin1String("u")));
    MegaHandle handle = INVALID_HANDLE;

    if (userHandle.size())
    {
        handle = MegaApi::base64ToUserHandle(userHandle.toLatin1().constData());
    }

    if (handle == ::mega::INVALID_HANDLE)
    {
        response = QString::number(MegaError::API_EARGS);
        MegaApi::log(MegaApi::LOG_LEVEL_DEBUG, "Add backup command received from the webclient: invalid handle");
    }
    else
    {
        MegaHandle appUser (megaApi->getMyUserHandleBinary());

        if (handle != appUser && appUser != ::mega::INVALID_HANDLE)
        {
            response = QString::number(MegaError::API_EACCESS);
            MegaApi::log(MegaApi::LOG_LEVEL_DEBUG, "Add backup command received from the webclient: user mismatch");
        }
        else
        {
            emit onExternalAddBackup();
            response = QString::number(MegaError::API_OK);
        }
    }
}

HTTPServer::RequestType HTTPServer::GetRequestType(const HTTPRequest &request)
{
    static const QString openLinkRequestStart(QLatin1String("{\"a\":\"l\","));
    static const QString externalDownloadRequestStart(QLatin1String("{\"a\":\"d\","));
    static const QString externalFileUploadRequestStart(QLatin1String("{\"a\":\"ufi\","));
    static const QString externalFolderUploadRequestStart(QLatin1String("{\"a\":\"ufo\","));
    static const QString externalFolderSyncRequestStart(QLatin1String("{\"a\":\"s\","));
    static const QString externalFolderSyncCheckStart(QLatin1String("{\"a\":\"sp\","));
    static const QString externalOpenTransferManagerStart(QLatin1String("{\"a\":\"tm\","));
    static const QString externalUploadSelectionStatusStart(QLatin1String("{\"a\":\"uss\","));
    static const QString externalTransferQueryProgressStart(QLatin1String("{\"a\":\"t\","));
    static const QString externalShowInFolder(QLatin1String("{\"a\":\"sf\","));
    static const QString versionCommand(QLatin1String("{\"a\":\"v\"}"));
    static const QString externalAddBackup(QLatin1String("{\"a\":\"ab\",\"u\":\""));

    if(request.data == versionCommand)
    {
        return VERSION_COMMAND;
    }
    else if(request.data.startsWith(openLinkRequestStart))
    {
        return OPEN_LINK_REQUEST_START;
    }
    else if(request.data.startsWith(externalDownloadRequestStart))
    {
        return EXTERNAL_DOWNLOAD_REQUEST_START;
    }
    else if(request.data.startsWith(externalFileUploadRequestStart))
    {
        return EXTERNAL_FILE_UPLOAD_REQUEST_START;
    }
    else if (request.data.startsWith(externalFolderUploadRequestStart))
    {
        return EXTERNAL_FOLDER_UPLOAD_REQUEST_START;
    }
    else if (request.data.startsWith(externalUploadSelectionStatusStart))
    {
        return EXTERNAL_UPLOAD_SELECTION_STATUS_START;
    }
    else if (request.data.startsWith(externalFolderSyncRequestStart))
    {
        return EXTERNAL_FOLDER_SYNC_REQUEST_START;
    }
    else if (request.data.startsWith(externalFolderSyncCheckStart))
    {
        return EXTERNAL_FOLDER_SYNC_CHECK_START;
    }
    else if(request.data.startsWith(externalOpenTransferManagerStart))
    {
        return EXTERNAL_OPEN_TRANSFER_MANAGER_START;
    }
    else if(request.data.startsWith(externalShowInFolder))
    {
        return EXTERNAL_SHOW_IN_FOLDER;
    }
    else if(request.data.startsWith(externalTransferQueryProgressStart))
    {
        return EXTERNAL_TRANSFER_QUERY_PROGRESS_START;
    }
    else if(request.data.startsWith(externalAddBackup))
    {
        return EXTERNAL_ADD_BACKUP;
    }
    return UNKNOWN_REQUEST;
}

QString HTTPServer::findCorrespondingAllowedOrigin(const QStringList& headers)
{
    for (const QString& allowedOrigin : qAsConst(Preferences::HTTPS_ALLOWED_ORIGINS))
    {
        QRegExp check = QRegExp(QString::fromUtf8("Origin: %1").arg(allowedOrigin),
                                Qt::CaseSensitive, QRegExp::Wildcard);
        for (const QString& header : headers)
        {
            if (check.exactMatch(header))
            {
               return header.mid(8);
            }
        }
    }
    return QString();
}

void HTTPServer::processPostRequest(QAbstractSocket *socket, HTTPRequest* request,
                                    const QStringList& headers, const QString& content)
{
    QString contentLengthId = QString::fromUtf8("Content-length: ");
    QStringList contentLengthHeader = headers.filter(QRegExp(contentLengthId, Qt::CaseInsensitive));
    if (!contentLengthHeader.size())
    {
        MegaApi::log(MegaApi::LOG_LEVEL_WARNING, "Missing Content-length header");
        rejectRequest(socket);
        return;
    }

    bool ok;
    request->contentLength = contentLengthHeader[0].mid(contentLengthId.size(), contentLengthHeader[0].size() - contentLengthId.size()).toInt(&ok);
    if (!ok || request->contentLength < content.size())
    {
        if (!ok)
        {
            MegaApi::log(MegaApi::LOG_LEVEL_WARNING, QString::fromUtf8("Unable to parse Content-length header: %1")
                         .arg(contentLengthHeader[0]).toUtf8().constData());
        }
        else
        {
            MegaApi::log(MegaApi::LOG_LEVEL_WARNING, QString::fromUtf8("Invalid Content-length header. Header: %1 - Data: %2")
                         .arg(request->contentLength).arg(content.size()).toUtf8().constData());
        }
        rejectRequest(socket);
        return;
    }

    if (request->contentLength > content.size())
    {
        return;
    }

    request->data = content;


    QPointer<QAbstractSocket> safeSocket = socket;
    QPointer<HTTPServer> safeServer = this;
    processRequest(socket, *request);
    if (!safeServer || !safeSocket)
    {
        return;
    }
}

void HTTPServer::sendPreFlightResponse(QAbstractSocket* socket, HTTPRequest* request, bool sendPrivateNetworkField)
{
    QPointer<QAbstractSocket> safeSocket = socket;
    QPointer<HTTPServer> safeServer = this;

    QString fullResponse = QString::fromUtf8("HTTP/1.1 204 No Content\r\n"
                                             "Server: MegaSync HTTP Server\r\n"
                                             "Access-Control-Allow-Origin: %1\r\n"
                                             "Access-Control-Allow-Methods: POST\r\n"
                                             ).arg(request->origin);
    if (sendPrivateNetworkField)
        fullResponse += QString::fromUtf8("Access-Control-Allow-Private-Network: true\r\n");

    fullResponse += QString::fromUtf8(   "Access-Control-Max-Age: 86400\r\n"
                                         "\r\n");

    if (safeServer && safeSocket)
    {
        safeSocket->write(fullResponse.toUtf8());
        safeSocket->flush();
        safeSocket->disconnectFromHost();
        safeSocket->deleteLater();
    }
}

void HTTPServer::processOptionRequest(QAbstractSocket* socket, HTTPRequest* request, const QStringList& headers)
{
    bool isCors = isPreFlightCorsRequest(headers);
    if (!isCors)
        return;

    bool hasPrivateNetworkField = hasFieldWithValue(headers, "Access-Control-Request-Private-Network", "true");

    QPointer<QAbstractSocket> safeSocket = socket;
    QPointer<HTTPServer> safeServer = this;

    sendPreFlightResponse(socket, request, hasPrivateNetworkField);

    if (!safeServer || !safeSocket)
    {
        return;
    }
}

bool HTTPServer::hasFieldWithValue(const QStringList& headers, const char* fieldName, const char* value)
{
    bool isFieldAsExpected = false;
    QString fieldNameStr = QString::fromUtf8(fieldName);
    QString keyString = QString::fromUtf8("%1: ").arg(fieldNameStr);
    QStringList foundValues = headers.filter(QRegExp(keyString, Qt::CaseInsensitive));
    if (foundValues.size() == 1)
    {
        QString foundValue = foundValues.front().mid(keyString.length());
        isFieldAsExpected = (foundValue == QString::fromUtf8(value));
    }
    else
    {
        const char* logString = (foundValues.size() > 1) ? "Several instances of field %1 in header"
                                                         : "field %1 not found in header";
        MegaApi::log(MegaApi::LOG_LEVEL_WARNING, QString::fromUtf8(logString).arg(fieldNameStr).toUtf8().constData());
    }

    return isFieldAsExpected;
}

bool HTTPServer::isPreFlightCorsRequest(const QStringList& headers)
{
    return hasFieldWithValue(headers, "Access-Control-Request-Method", "POST");
}

bool HTTPServer::isRequestOfType(const QStringList& headers, const char* typeName)
{
    return headers[0].startsWith(QString::fromAscii(typeName));
}
