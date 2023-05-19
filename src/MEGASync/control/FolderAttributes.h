#ifndef FOLDERATTRIBUTES_H
#define FOLDERATTRIBUTES_H

#include <QTMegaRequestListener.h>

#include <QDateTime>
#include <QFutureWatcher>

#include <functional>

class FileFolderAttributes : public QObject
{
    Q_OBJECT

public:
    FileFolderAttributes(QObject* parent);
    virtual ~FileFolderAttributes();

    virtual void requestSize(QObject* caller,std::function<void(qint64)> func);
    virtual void requestModifiedTime(QObject *caller, std::function<void(const QDateTime&)> func);
    virtual void requestCreatedTime(QObject *caller, std::function<void(const QDateTime&)> func);

    void cancel();

signals:
    void sizeReady(qint64);
    void modifiedTimeReady(const QDateTime&);
    void createdTimeReady(const QDateTime&);

protected:
    bool mCancelled;

    qint64 mSize;
    QDateTime mModifiedTime;
    QDateTime mCreatedTime;
};

class LocalFileFolderAttributes : public FileFolderAttributes
{
    Q_OBJECT

public:
    LocalFileFolderAttributes(const QString& path, QObject* parent);
    ~LocalFileFolderAttributes() override = default;

    void requestSize(QObject* caller,std::function<void(qint64)> func) override;
    void requestModifiedTime(QObject* caller,std::function<void(const QDateTime&)> func) override;
    void requestCreatedTime(QObject* caller,std::function<void(const QDateTime&)> func) override;

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

class RemoteFileFolderAttributes : public FileFolderAttributes, public mega::MegaRequestListener
{
    Q_OBJECT

public:
    RemoteFileFolderAttributes(mega::MegaHandle handle, QObject *parent);
    ~RemoteFileFolderAttributes() override;

    void requestSize(QObject* caller,std::function<void(qint64)> func) override;
    void requestModifiedTime(QObject* caller,std::function<void(const QDateTime&)> func) override;
    void requestCreatedTime(QObject* caller,std::function<void(const QDateTime&)> func) override;

    void onRequestFinish(mega::MegaApi *api, mega::MegaRequest *request, mega::MegaError *e) override;

private:
    mega::QTMegaRequestListener* mListener;
    mega::MegaHandle mHandle;
};

#endif // FOLDERATTRIBUTES_H
