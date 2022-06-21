#ifndef MACXLOCALSOCKET_H
#define MACXLOCALSOCKET_H

#include <QObject>
#include <objc/runtime.h>

class MacXLocalSocketPrivate;
class MacXLocalSocket : public QObject
{
    Q_OBJECT

public:
    MacXLocalSocket(MacXLocalSocketPrivate *clientSocketPrivate);
    ~MacXLocalSocket();

    qint64 readCommand(QByteArray *data);

    //This method is called from two different threads, but it is thread-safe
    qint64 writeData(const char * data, qint64 len);

signals:
    void dataReady();
    void disconnected();

private:
    MacXLocalSocketPrivate* socketPrivate;
};

#endif // MACXLOCALSOCKET_H
