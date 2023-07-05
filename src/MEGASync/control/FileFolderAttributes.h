#ifndef FILEFOLDERATTRIBUTES_H
#define FILEFOLDERATTRIBUTES_H

#include <QTMegaRequestListener.h>

#include <QDateTime>
#include <QFutureWatcher>

#include <functional>
#include <memory>

namespace UserAttributes{
class FullName;
}

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

    template <class Type>
    static std::shared_ptr<Type> convert(std::shared_ptr<FileFolderAttributes> attributes)
    {
        return std::dynamic_pointer_cast<Type>(attributes);
    }



    qint64 size() const;
    int64_t modifiedTime() const;
    int64_t createdTime() const;

signals:
    void sizeReady(qint64);
    void modifiedTimeReady(const QDateTime&);
    void createdTimeReady(const QDateTime&);

protected:
    enum AttributeTypes
    {
        Size = 0,
        ModifiedTime,
        CreatedTime,
        LocalAttributes = 10,
        RemoteAttributes = 20,
        Last
    };

    bool mCancelled;

    qint64 mSize;
    QDateTime mModifiedTime;
    QDateTime mCreatedTime;

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
    RemoteFileFolderAttributes(const QString& filePath, QObject *parent);
    RemoteFileFolderAttributes(mega::MegaHandle handle, QObject *parent);

    ~RemoteFileFolderAttributes() override;

    void requestSize(QObject* caller,std::function<void(qint64)> func) override;
    void requestModifiedTime(QObject* caller,std::function<void(const QDateTime&)> func) override;
    void requestCreatedTime(QObject* caller,std::function<void(const QDateTime&)> func) override;
    void requestUser(QObject* caller, std::function<void(QString, bool)> func);
    void requestUser(QObject* caller, mega::MegaHandle currentUser, std::function<void(QString, bool)> func);
    void requestVersions(QObject*, std::function<void(int)> func);

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
};

#endif // FILEFOLDERATTRIBUTES_H
