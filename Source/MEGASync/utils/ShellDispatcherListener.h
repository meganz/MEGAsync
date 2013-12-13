#ifndef SHELLDISPATCHERLISTENER_H
#define SHELLDISPATCHERLISTENER_H

#include <QQueue>
#include <QString>

class ShellDispatcherListener {

public:
    virtual void shellUpload(QQueue<QString> uploads) = 0;
};

#endif // SHELLDISPATCHERLISTENER_H
