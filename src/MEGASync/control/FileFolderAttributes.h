#ifndef FILEFOLDERATTRIBUTES_H
#define FILEFOLDERATTRIBUTES_H

#include <QTMegaRequestListener.h>

#include <QDateTime>
#include <QFutureWatcher>

#include <functional>
#include <memory>

#include <QEventLoop>

namespace UserAttributes{
class FullName;
}

class FileFolderAttributes : public QObject
{
    Q_OBJECT

public:

    enum Status
    {
        NOT_READY = -1,
        NOT_READABLE = -2
    };

    FileFolderAttributes(QObject* parent);
    virtual ~FileFolderAttributes();

    virtual void initAllAttributes();

    template <typename AttributeType, typename VariableType, typename Func1>
    void requestCustomValue(AttributeType* sender, QObject* caller,std::function<void(VariableType)> func, Func1 signal, int type)
    {
        if(auto context = requestReady(type, caller))
        {
            connect(sender, signal, context, [this, func, type](VariableType value){
                if(func)
                {
                    func(value);
                }
                requestFinish(type);
            });
        }
    }

    virtual void requestSize(QObject* caller,std::function<void(qint64)> func);
    virtual void requestModifiedTime(QObject *caller, std::function<void(const QDateTime&)> func);
    virtual void requestCreatedTime(QObject *caller, std::function<void(const QDateTime&)> func);
    virtual void requestCRC(QObject* caller,std::function<void(const QString&)> func);

    void cancel();

    template <class Type>
    static std::shared_ptr<Type> convert(std::shared_ptr<FileFolderAttributes> attributes)
    {
        return std::dynamic_pointer_cast<Type>(attributes);
    }

    int64_t size() const;
    int64_t modifiedTime() const;
    int64_t createdTime() const;
    const QString& fingerprint() const;

signals:
    void sizeReady(qint64);
    void modifiedTimeReady(const QDateTime&);
    void createdTimeReady(const QDateTime&);
    void CRCReady(const QString&);

    void attributesChanged();

protected:
    enum AttributeTypes
    {
        Size = 0,
        ModifiedTime,
        CreatedTime,
        CRC,
        FileCount,
        LocalAttributes = 10,
        RemoteAttributes = 20,
        Last
    };

    bool mCancelled;

    qint64 mSize;
    QDateTime mModifiedTime;
    QDateTime mCreatedTime;
    QString mFp;

    template <class ValueType>
    void setValue(const ValueType& newValue, ValueType& member)
    {
        if(member != newValue)
        {
            member = newValue;
            emit attributesChanged();
        }
    }

    bool attributeNeedsUpdate(int type);
    QObject* requestReady(int type, QObject* caller);
    void requestFinish(int type);
    QMap<int, QPointer<QObject>> mRequests;
    QMap<int, int64_t> mRequestTimestamps;
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
    void requestCRC(QObject* caller,std::function<void(const QString&)> func) override;

    void setPath(const QString &newPath);

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

class RemoteFileFolderAttributes : public FileFolderAttributes
{
    Q_OBJECT

public:
    RemoteFileFolderAttributes(const QString& filePath, QObject *parent, bool waitForAttributes);
    RemoteFileFolderAttributes(mega::MegaHandle handle, QObject *parent, bool waitForAttributes);

    ~RemoteFileFolderAttributes() override;

    void initAllAttributes() override;

    void requestSize(QObject* caller,std::function<void(qint64)> func) override;
    void requestFileCount(QObject* caller, std::function<void (int)> func);
    void requestModifiedTime(QObject* caller,std::function<void(const QDateTime&)> func) override;
    void requestCreatedTime(QObject* caller,std::function<void(const QDateTime&)> func) override;
    void requestCRC(QObject* caller,std::function<void(const QString&)> func) override;
    void requestUser(QObject* caller, std::function<void(QString, bool)> func);
    void requestUser(QObject* caller, mega::MegaHandle currentUser, std::function<void(QString, bool)> func);
    void requestVersions(QObject*, std::function<void(int)> func);

    int versionCount();
    int fileCount();

    void setHandle(mega::MegaHandle newHandle);

signals:
    void fileCountReady(int);

private:
    enum class Version
    {
        First,
        Last
    };
    std::unique_ptr<mega::MegaNode> getNode(Version type = Version::Last) const;

    QString mFilePath;
    mega::MegaHandle mHandle;

    enum RemoteAttributeTypes
    {
        User = AttributeTypes::RemoteAttributes,
        Versions
    };

    QString mUserEmail;
    mega::MegaHandle mOwner = mega::INVALID_HANDLE;
    std::shared_ptr<const UserAttributes::FullName> mUserFullName;
    int mVersionCount;
    int mFileCount;

    bool mWaitForAttributes;

    QEventLoop mEventLoop;
};

#endif // FILEFOLDERATTRIBUTES_H
