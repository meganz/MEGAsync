#ifndef MACXLOCALSOCKET_H
#define MACXLOCALSOCKET_H

#include <QObject>
#include <objc/runtime.h>

class MacXLocalSocketPrivate;
class MacXLocalSocket : public QObject
{
    Q_OBJECT

public:
    MacXLocalSocket(QObject *parent, MacXLocalSocketPrivate *clientSocketPrivate);
    ~MacXLocalSocket();

    qint64 readCommand(QByteArray *data);
    qint64 writeData(const char * data, qint64 len);

signals:
    void dataReady();
    void disconnected();

private:
    QScopedPointer<MacXLocalSocketPrivate> socketPrivate;
};

#endif // MACXLOCALSOCKET_H
