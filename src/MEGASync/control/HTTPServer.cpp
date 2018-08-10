#include "HTTPServer.h"
#include "Preferences.h"
#include "Utilities.h"
#include "MegaApplication.h"

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
}

QMultiMap<QString, RequestData*> HTTPServer::webDataRequests;
QMap<mega::MegaHandle, RequestTransferData*> HTTPServer::webTransferStateRequests;

HTTPServer::HTTPServer(MegaApi *megaApi, quint16 port, bool sslEnabled)
    : QTcpServer(), disabled(false)
{
    this->megaApi = megaApi;
    this->sslEnabled = sslEnabled;
    this->isFirstWebDownloadDone = false;
    listen(QHostAddress::LocalHost, port);
}

HTTPServer::~HTTPServer()
{

}

#if QT_VERSION >= 0x050000
void HTTPServer::incomingConnection(qintptr socket)
#else
void HTTPServer::incomingConnection(int socket)
#endif
{
    if (disabled)
    {
        return;
    }

    Preferences *preferences = Preferences::instance();
    QTcpSocket* s = NULL;
    QSslSocket *sslSocket = NULL;

    if (sslEnabled)
    {
        sslSocket = new QSslSocket(this);;
        s = sslSocket;
        connect(sslSocket, SIGNAL(sslErrors(QList<QSslError>)), this, SLOT(sslErrors(QList<QSslError>)));
        connect(sslSocket, SIGNAL(peerVerifyError(QSslError)), this, SLOT(peerVerifyError(QSslError)));
    }
    else
    {
        s = new QTcpSocket(this);
    }

    connect(s, SIGNAL(readyRead()), this, SLOT(readClient()));
    connect(s, SIGNAL(disconnected()), this, SLOT(discardClient()));
    connect(s, SIGNAL(error(QAbstractSocket::SocketError)), this, SLOT(error(QAbstractSocket::SocketError)));

    s->setSocketDescriptor(socket);
    requests.insert(s, new HTTPRequest());

    if (sslSocket)
    {
        sslSocket->setPeerVerifyMode(QSslSocket::VerifyNone);

        QSslKey key(preferences->getHttpsKey().toUtf8(), QSsl::Rsa, QSsl::Pem, QSsl::PrivateKey);
        if (key.isNull())
        {
            s->disconnectFromHost();
            return;
        }

#if QT_VERSION >= 0x050100
        QList<QSslCertificate> certificates;
        certificates.append(QSslCertificate(preferences->getHttpsCert().toUtf8(), QSsl::Pem));
        QStringList intermediates = preferences->getHttpsCertIntermediate().split(QString::fromUtf8(";"), QString::SkipEmptyParts);
        for (int i = 0; i < intermediates.size(); i++)
        {
            certificates.append(QSslCertificate(intermediates.at(i).toUtf8(), QSsl::Pem));
        }
        sslSocket->setLocalCertificateChain(certificates);
#else
        sslSocket->setLocalCertificate(QSslCertificate(preferences->getHttpsCert().toUtf8(), QSsl::Pem));
#endif
        sslSocket->setPrivateKey(key);
        sslSocket->startServerEncryption();
    }
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

void HTTPServer::onTransferDataUpdate(MegaHandle handle, int state, long long progress, long long size, long long speed)
{
    QMap<MegaHandle, RequestTransferData*>::iterator it = webTransferStateRequests.find(handle);
    if (it == webTransferStateRequests.end())
    {
        return;
    }

    RequestTransferData* tData = it.value();
    tData->state = state;
    tData->progress = progress;
    tData->size = size;
    tData->speed = speed;
    if (state == MegaTransfer::STATE_CANCELLED
            || state == MegaTransfer::STATE_COMPLETED
            || state == MegaTransfer::STATE_FAILED)
    {
        tData->tsEnd = QDateTime::currentMSecsSinceEpoch() / 1000;
    }
}

void HTTPServer::readClient()
{
    QAbstractSocket *socket = (QSslSocket*)sender();
    HTTPRequest *request = requests.value(socket);
    if (disabled || !request)
    {
        discardClient();
        return;
    }

    request->data.append(QString::fromUtf8(socket->readAll().data()));
    if (request->data.contains(QString::fromUtf8("\r\n\r\n")))
    {
        QStringList tokens = request->data.split(QString::fromUtf8("\r\n\r\n"));
        QStringList headers = tokens[0].split(QString::fromUtf8("\r\n"));
        if (!headers.size() || !headers[0].startsWith(QString::fromAscii("POST")))
        {
            rejectRequest(socket, QString::fromUtf8("405 Method Not Allowed"));
            return;
        }

        if (Preferences::HTTPS_ORIGIN_CHECK_ENABLED && !Preferences::HTTPS_ALLOWED_ORIGINS.isEmpty())
        {
            bool found = false;
            for (int i = 0; i < Preferences::HTTPS_ALLOWED_ORIGINS.size(); i++)
            {                
                QRegExp check = QRegExp(QString::fromUtf8("Origin: %1").arg(Preferences::HTTPS_ALLOWED_ORIGINS.at(i)),
                                        Qt::CaseSensitive, QRegExp::Wildcard);
                for (int j = 0; j < headers.size(); j++)
                {
                    if (check.exactMatch(headers[j]))
                    {
                       request->origin = headers[j].mid(8);
                       found = true;
                       break;
                    }
                }

                if (found)
                {
                    break;
                }
            }

            if (!found)
            {
                rejectRequest(socket);
                return;
            }
        }

        QString contentLengthId = QString::fromUtf8("Content-length: ");
        QStringList contentLengthHeader = headers.filter(QRegExp(contentLengthId, Qt::CaseInsensitive));
        if (!contentLengthHeader.size())
        {
            rejectRequest(socket);
            return;
        }

        bool ok;
        request->contentLength = contentLengthHeader[0].mid(contentLengthId.size(), contentLengthHeader[0].size() - contentLengthId.size()).toInt(&ok);
        if (!ok || request->contentLength < tokens[1].size())
        {
            rejectRequest(socket);
            return;
        }

        if (request->contentLength > tokens[1].size())
        {
            return;
        }

        request->data = tokens[1];
        processRequest(socket, *request);

        HTTPRequest *req = requests.value(socket, NULL);
        if (request == req)
        {
            requests.remove(socket);
            delete request;
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

void HTTPServer::processRequest(QAbstractSocket *socket, HTTPRequest request)
{
    QString response;
    QString openLinkRequestStart(QString::fromUtf8("{\"a\":\"l\","));
    QString externalDownloadRequestStart   = QString::fromUtf8("{\"a\":\"d\",");
    QString externalFileUploadRequestStart = QString::fromUtf8("{\"a\":\"ufi\",");
    QString externalFolderUploadRequestStart = QString::fromUtf8("{\"a\":\"ufo\",");
    QString externalFolderSyncRequestStart = QString::fromUtf8("{\"a\":\"s\",");
    QString externalFolderSyncCheckStart   = QString::fromUtf8("{\"a\":\"sp\",");
    QString externalOpenTransferManagerStart   = QString::fromUtf8("{\"a\":\"tm\",");
    QString externalUploadSelectionStatusStart = QString::fromUtf8("{\"a\":\"uss\",");
    QString externalTransferQueryProgressStart = QString::fromUtf8("{\"a\":\"t\",");

    QPointer<QAbstractSocket> safeSocket = socket;

    MegaApi::log(MegaApi::LOG_LEVEL_DEBUG, QString::fromUtf8("HTTP request received: %1").arg(request.data).toUtf8().constData());
    if (request.data == QString::fromUtf8("{\"a\":\"v\"}"))
    {
        MegaApi::log(MegaApi::LOG_LEVEL_DEBUG, "GetVersion command received from the webclient");
        char *myHandle = megaApi->getMyUserHandle();
        if (!myHandle)
        {
            response = QString::fromUtf8("{\"v\":\"%1\"}").arg(Preferences::VERSION_STRING);
        }
        else
        {
            response = QString::fromUtf8("{\"v\":\"%1\",\"u\":\"%2\"}")
                    .arg(Preferences::VERSION_STRING)
                    .arg(QString::fromUtf8(myHandle));
            delete [] myHandle;
        }
    }
    else if (request.data.startsWith(openLinkRequestStart))
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
            QString link = QString::fromUtf8("https://mega.nz/#!%1!%2").arg(handle).arg(key);
            emit onLinkReceived(link, auth);
            response = QString::fromUtf8("0");

            Preferences *preferences = Preferences::instance();
            QString defaultPath = preferences->downloadFolder();
            webTransferStateRequests.insert(megaApi->base64ToHandle(handle.toUtf8().constData()), new RequestTransferData());

            if (preferences->hasDefaultDownloadFolder() && QFile(defaultPath).exists())
            {
                ((MegaApplication *)qApp)->showInfoMessage(tr("Your download has started"));
            }

            if (!isFirstWebDownloadDone && !Preferences::instance()->isFirstWebDownloadDone())
            {
                megaApi->sendEvent(99503, "MEGAsync first webclient download");
                isFirstWebDownloadDone = true;
            }
        }
        else if (key.size() && key.size() != 43)
        {
            response = QString::number(MegaError::API_EKEY);
        }
    }
    else if (request.data.startsWith(externalDownloadRequestStart))
    {
        MegaApi::log(MegaApi::LOG_LEVEL_DEBUG, "ExternalDownload command received from the webclient");
        int start = request.data.indexOf(QString::fromUtf8("\"f\":[")) + 5;
        if (start > 0)
        {
            QString privateAuth = Utilities::extractJSONString(request.data.mid(0, start), QString::fromUtf8("esid"));
            QString publicAuth = Utilities::extractJSONString(request.data.mid(0, start), QString::fromUtf8("en"));

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
                QQueue<MegaNode *> downloadQueue;
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

                    int type = Utilities::extractJSONNumber(file, QString::fromUtf8("t"));
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
                    }
                    else
                    {
                        firstnode = false;
                    }

                    if (type != MegaNode::TYPE_FILE)
                    {
                        MegaNode *node = megaApi->createForeignFolderNode(h, name.toUtf8().constData(), p,
                                                                         privateAuth.toUtf8().constData(),
                                                                         publicAuth.toUtf8().constData());
                        downloadQueue.append(node);
                    }
                    else
                    {
                        QString key = Utilities::extractJSONString(file, QString::fromUtf8("k"));
                        if (key.size() == 43)
                        {
                            long long size = Utilities::extractJSONNumber(file, QString::fromUtf8("s"));
                            long long mtime = Utilities::extractJSONNumber(file, QString::fromUtf8("ts"));

                            MegaNode *node = megaApi->createForeignFileNode(h, key.toUtf8().constData(),
                                                             name.toUtf8().constData(), size, mtime,
                                                             p, privateAuth.toUtf8().constData(),
                                                             publicAuth.toUtf8().constData());
                            downloadQueue.append(node);
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
    else if (request.data.startsWith(externalFileUploadRequestStart))
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
    else if (request.data.startsWith(externalFolderUploadRequestStart))
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
    else if (request.data.startsWith(externalUploadSelectionStatusStart))
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
    else if (request.data.startsWith(externalFolderSyncRequestStart))
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
    else if (request.data.startsWith(externalFolderSyncCheckStart))
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
                int result = megaApi->isNodeSyncable(targetNode);
                response = QString::number(result);
                delete targetNode;
            }
        }
        else
        {
            response = QString::number(MegaError::API_EARGS);
        }
    }
    else if (request.data.startsWith(externalOpenTransferManagerStart))
    {
        MegaApi::log(MegaApi::LOG_LEVEL_DEBUG, "Open Transfer Manager command received from the webclient");
        int tab = Utilities::extractJSONNumber(request.data, QString::fromUtf8("t"));
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
    else if (request.data.startsWith(externalTransferQueryProgressStart))
    {
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
    if (safeSocket)
    {
        safeSocket->write(fullResponse.toUtf8());
        safeSocket->flush();
        safeSocket->disconnectFromHost();
        safeSocket->deleteLater();
    }
}

void HTTPServer::error(QAbstractSocket::SocketError)
{
}

void HTTPServer::sslErrors(const QList<QSslError> &)
{
}

void HTTPServer::peerVerifyError(const QSslError &)
{
}
