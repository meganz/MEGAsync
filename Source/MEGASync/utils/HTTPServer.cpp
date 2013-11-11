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
     connect(s, SIGNAL(readyRead()), this, SLOT(readClient()));
     connect(s, SIGNAL(disconnected()), this, SLOT(discardClient()));
     s->setSocketDescriptor(socket);
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
     QTcpSocket* socket = (QTcpSocket*)sender();
     if (socket->canReadLine()) {
         QStringList tokens = QString(socket->readLine()).split(QRegExp("[ \r\n][ \r\n]*"));
         if (tokens[0] == "GET") {
             QTextStream os(socket);
             os.setAutoDetectUnicode(true);
             os << "HTTP/1.0 200 Ok\r\n"
                 "Content-Type: text/html; charset=\"utf-8\"\r\n"
                 "\r\n"
                 "<h1>Nothing to see here</h1>\n"
                 << QDateTime::currentDateTime().toString() << "\n";
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
