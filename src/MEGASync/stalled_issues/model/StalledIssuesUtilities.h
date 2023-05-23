#ifndef STALLEDISSUESUTILITIES_H
#define STALLEDISSUESUTILITIES_H

#include <QTMegaRequestListener.h>

#include <QObject>
#include <QString>
#include <QFutureWatcher>
#include <QFileInfo>

#include <memory>

class StalledIssuesUtilities : public QObject , public mega::MegaRequestListener
{
    Q_OBJECT

public:
    StalledIssuesUtilities();

    void ignoreFile(const QString& path);
    void removeRemoteFile(const QString& path);
    void removeLocalFile(const QString& path);

    static QIcon getLocalFileIcon(const QFileInfo& fileInfo, bool hasProblem);
    static QIcon getRemoteFileIcon(mega::MegaNode* node, const QFileInfo &fileInfo, bool hasProblem);

signals:
    void actionFinished();

protected slots:
    void onRequestFinish(mega::MegaApi *, mega::MegaRequest *request, mega::MegaError *e);

private slots:
    void onIgnoreFileFinished();

private:
    static QIcon getFileIcon(bool isFile, const QFileInfo &fileInfo, bool hasProblem);

    QFutureWatcher<void> mIgnoreWatcher;

    std::unique_ptr<mega::QTMegaRequestListener> mListener;
    mega::MegaHandle mRemoteHandle;
};

#endif // STALLEDISSUESUTILITIES_H
