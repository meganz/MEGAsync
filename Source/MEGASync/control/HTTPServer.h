#ifndef HTTPSERVER_H
#define HTTPSERVER_H

#include <QTcpServer>
#include <QTcpSocket>
#include <QStringList>
#include <QDateTime>

class HTTPServer: public QTcpServer
{
    Q_OBJECT

    public:
        HTTPServer(quint16 port, QObject* parent = 0);
        void incomingConnection(int socket);
        void pause();
        void resume();

    private slots:
        void readClient();
        void discardClient();

    private:
        bool disabled;
};

#endif // HTTPSERVER_H
