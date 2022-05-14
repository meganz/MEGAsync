#ifndef STALLEDISSUESUTILITIES_H
#define STALLEDISSUESUTILITIES_H

#include <QTMegaRequestListener.h>

#include <QObject>
#include <QString>
#include <QFutureWatcher>

#include <memory>

class StalledIssuesUtilities : public QObject , public mega::MegaRequestListener
{
    Q_OBJECT

public:
    StalledIssuesUtilities();

    void ignoreFile(const QString& path);
    void removeRemoteFile(const QString& path);
    void removeLocalFile(const QString& path);
    void renameCloudFile(const QString& path, const QString& newFile);
    void renameLocalFile(const QString& path, const QString& newFile);

signals:
    void actionFinished();

protected slots:
    void onRequestFinish(mega::MegaApi *, mega::MegaRequest *request, mega::MegaError *e);

private slots:
    void onIgnoreFileFinished();

private:
    QFutureWatcher<void> mIgnoreWatcher;

    std::unique_ptr<mega::QTMegaRequestListener> mListener;
    mega::MegaHandle mRemoteHandle;
};

#endif // STALLEDISSUESUTILITIES_H
