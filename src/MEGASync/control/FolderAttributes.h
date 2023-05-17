#ifndef FOLDERATTRIBUTES_H
#define FOLDERATTRIBUTES_H

#include <QTMegaRequestListener.h>

#include <QDateTime>
#include <QFutureWatcher>

class FolderAttributes : public QObject
{
    Q_OBJECT

public:
    FolderAttributes(QObject* parent);
    virtual ~FolderAttributes();

    virtual void requestSize() = 0;
    virtual void requestModifiedTime() = 0;
    virtual void requestCreatedTime() = 0;

    void cancel();

signals:
    void sizeReady(qint64 size);
    void modifiedTimeReady(QDateTime time);
    void createdTimeReady(QDateTime time);

protected:
    bool mCancelled;

    qint64 mSize;
    QDateTime mModifiedTime;
    QDateTime mCreatedTime;
};

class LocalFolderAttributes : public FolderAttributes
{
    Q_OBJECT

public:
    LocalFolderAttributes(const QString& path, QObject* parent);
    ~LocalFolderAttributes() override = default;

    void requestSize() override;
    void requestModifiedTime() override;
    void requestCreatedTime() override;

private slots:
    void onModifiedTimeCalculated();
    void onSizeCalculated();
    
private:
    QDateTime calculateModifiedTime();
    qint64 calculateSize();

    QFutureWatcher<qint64> mFolderSizeFuture;
    QFutureWatcher<QDateTime> mModifiedTimeWatcher;
    QString mPath;
    bool mIsEmpty;
};

class RemoteFolderAttributes : public FolderAttributes, public mega::MegaRequestListener
{
    Q_OBJECT

public:
    RemoteFolderAttributes(mega::MegaHandle handle, QObject *parent);
    ~RemoteFolderAttributes() override;

    void requestSize() override;
    void requestModifiedTime() override;
    void requestCreatedTime() override;

    void onRequestFinish(mega::MegaApi *api, mega::MegaRequest *request, mega::MegaError *e) override;

private:
    mega::QTMegaRequestListener* mListener;
    mega::MegaHandle mHandle;
};

#endif // FOLDERATTRIBUTES_H
