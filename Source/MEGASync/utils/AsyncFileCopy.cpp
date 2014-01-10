#include "AsyncFileCopy.h"

#include <QFile>

AsyncFileCopy::AsyncFileCopy(QString srcFile, QString dstFile, QObject *parent) :
    QObject(parent)
{
    this->srcFile = srcFile;
    this->dstFile = dstFile;
}

void AsyncFileCopy::doWork()
{
    QFile file(srcFile);
    QFile dst(dstFile);
    dst.remove();
    file.copy(dstFile);
}
