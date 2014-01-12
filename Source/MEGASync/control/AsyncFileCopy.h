#ifndef ASYNCFILECOPY_H
#define ASYNCFILECOPY_H

#include <QObject>

class AsyncFileCopy : public QObject
{
    Q_OBJECT
public:
    explicit AsyncFileCopy(QString srcFile, QString dstFile, QObject *parent = 0);

signals:

public slots:
    void doWork();

protected:
    QString srcFile;
    QString dstFile;
};

#endif // ASYNCFILECOPY_H
