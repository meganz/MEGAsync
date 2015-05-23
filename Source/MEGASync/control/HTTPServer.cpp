#include "HTTPServer.h"

HTTPServer::HTTPServer(quint16 port, QObject* parent)
    : QTcpServer(parent), disabled(false)
{
    listen(QHostAddress::LocalHost, port);
}

 void HTTPServer::incomingConnection(int socket)
 {
     if (disabled)
         return;

     // When a new client connects, the server constructs a QTcpSocket and all
     // communication with the client is done over this QTcpSocket. QTcpSocket
     // works asynchronously, this means that all the communication is done
     // in the two slots readClient() and discardClient().
     QTcpSocket* s = new QTcpSocket(this);
     //QSslSocket *s = new QSslSocket(this);

     connect(s, SIGNAL(readyRead()), this, SLOT(readClient()));
     connect(s, SIGNAL(disconnected()), this, SLOT(discardClient()));
     s->setSocketDescriptor(socket);

     /*
     QFile file(PRIVATEKEY_FILE);
     file.open(QIODevice::ReadOnly);
     if (!file.isOpen()) {
        qWarning("could'n open %s", PRIVATEKEY_FILE);
        a->disconnectFromHost();
        return;
     }

     QSslKey key(&file, QSsl::Rsa, QSsl::Pem, QSsl::PrivateKey, "server");

     if (key.isNull())
     {
        qWarning("key is null");
        s->disconnectFromHost();
        return;
     }

     if(!s->addCaCertificates(CACERTIFICATES_FILE))
     {
        qWarning("Couldn't add CA certificates (\"%s\")" , CACERTIFICATES_FILE);
     }
     else
     {
        socket->setLocalCertificate(LOCALCERTIFICATE_FILE);
        socket->setPrivateKey(key);
        socket->startServerEncryption();
     }*/
 }

 void HTTPServer::pause()
 {
     disabled = true;
 }

 void HTTPServer::resume()
 {
     disabled = false;
 }

 void HTTPServer::readClient()
 {
     if (disabled)
         return;

     // This slot is called when the client sent data to the server. The
     // server looks if it was a get request and sends a very simple HTML
     // document back.

     bool valid = false;
     QTcpSocket* socket = (QTcpSocket*)sender();
     request.append(QString::fromUtf8(socket->readAll().data()));
     if (request.contains(QString::fromUtf8("\r\n\r\n")))
     {
         QStringList tokens = request.split(QString::fromUtf8("\r\n\r\n"));
         QStringList headers = tokens[0].split(QString::fromUtf8("\r\n"));
         if (headers[0].startsWith(QString::fromAscii("GET")))
         {
             QStringList result = headers.filter(QRegExp(QString::fromUtf8("Origin: http://mega.nz"), Qt::CaseInsensitive));
             if(result.size())
             {
                 valid = true;
             }

             QTextStream os(socket);
             os.setAutoDetectUnicode(true);
             if(!valid)
             {
                 os << "HTTP/1.0 403 Forbidden\r\n"
                     "Content-Type: text/html; charset=\"utf-8\"\r\n"
                     "\r\n";
             }
             else
             {
                 os << "HTTP/1.0 200 Ok\r\n"
                     "Access-Control-Allow-Origin: http://mega.nz\r\n"
                     "Content-Type: text/html; charset=\"utf-8\"\r\n"
                     "\r\n"
                     "OK!\n";
             }
             socket->close();

             if (socket->state() == QTcpSocket::UnconnectedState) {
                 delete socket;
             }
         }
     }
 }
 void HTTPServer::discardClient()
 {
     QTcpSocket* socket = (QTcpSocket*)sender();
     socket->deleteLater();
 }
