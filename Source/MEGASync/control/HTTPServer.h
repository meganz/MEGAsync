#ifndef HTTPSERVER_H
#define HTTPSERVER_H

#include <QTcpServer>
#include <QSslSocket>
#include <QSslKey>
#include <QFile>
#include <QStringList>
#include <QDateTime>

class HTTPRequest
{
public:
    HTTPRequest() : contentLength(0) {}
    QString data;
    int contentLength;
};

class HTTPServer: public QTcpServer
{
    Q_OBJECT

    public:
        HTTPServer(quint16 port, bool sslEnabled);
#if QT_VERSION >= 0x050000
        void incomingConnection(qintptr socket);
#else
        void incomingConnection(int socket);
#endif
        void pause();
        void resume();

    signals:
        void onLinkReceived(QString link);

    private slots:
        void readClient();
        void discardClient();
        void rejectRequest(QAbstractSocket *socket, QString response = QString::fromUtf8("403 Forbidden"));
        void processRequest(QAbstractSocket *socket, HTTPRequest *request);
        void error(QAbstractSocket::SocketError);

    private:
        bool disabled;
        bool sslEnabled;
        QMap<QAbstractSocket*, HTTPRequest*> requests;
};

#endif // HTTPSERVER_H
