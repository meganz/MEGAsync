#ifndef FILEFOLDERATTRIBUTES_H
#define FILEFOLDERATTRIBUTES_H

#include "megaapi.h"
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

protected:
    enum AttributeTypes
    {
        SIZE = 0,
        MODIFIED_TIME,
        CREATED_TIME,
        CRC,
        LOCAL_ATTRIBUTES = 10,
        REMOTE_ATTRIBUTES = 20,
        LAST
    };

public:
    enum Status
    {
        NOT_READY = -1,
        NOT_READABLE = -2
    };

    FileFolderAttributes(QObject* parent);
    virtual ~FileFolderAttributes();

    virtual void initAllAttributes();
    void setValueUpdatesDisable();
    bool areValueUpdatesDisabled() const;

    virtual void requestSize(QObject*, std::function<void(qint64)>) = 0;
    virtual void requestModifiedTime(QObject*, std::function<void(const QDateTime&)>) = 0;
    virtual void requestCreatedTime(QObject*, std::function<void(const QDateTime&)>) = 0;
    virtual void requestCRC(QObject*, std::function<void(const QString&)>) = 0;

    void cancel();

    template <class Type>
    static std::shared_ptr<Type> convert(std::shared_ptr<FileFolderAttributes> attributes)
    {
        return std::dynamic_pointer_cast<Type>(attributes);
    }

    int64_t size() const;
    int64_t modifiedTimeInSecs() const;
    QDateTime modifiedTime() const;
    int64_t createdTimeInSecs() const;
    QDateTime createdTime() const;
    QString getCRC() const;

signals:
    void attributeReady(int type, bool isPlaceHolder = false);
    void attributesChanged();

protected:
    virtual bool attributeNeedsUpdate(QObject* caller, int type);
    QObject* requestReady(int type, QObject* caller);
    void requestFinish(int type);

    template <class ValueType>
    void initValue(int type, ValueType value)
    {
        if(!mValues.contains(type))
        {
            mValues.insert(type, value);
        }
    }

    template <class ValueType>
    bool requestValue(QObject* caller, int type, std::function<void(const ValueType&)> func)
    {
        if (attributeNeedsUpdate(caller, type))
        {
            if (auto context = requestReady(type, caller))
            {
                connect(this,
                    &FileFolderAttributes::attributeReady,
                    context,
                    [this, type, func](int signalType, bool isPlaceHolder) {
                        if (signalType == type && func)
                        {
                            auto value(mValues.value(type).value<ValueType>());
                            func(value);

                            if (!isPlaceHolder)
                            {
                                requestFinish(type);
                            }
                        }
                    });
            }
            return true;
        }
        else if (func)
        {
            // send the current value as we don´t need to update it (it was updated at most 2
            // seconds ago)
            func(mValues.value(type).value<ValueType>());
        }

        return false;
    }

    bool mCancelled;
    QMap<int, QPointer<QObject>> mRequests;
    QMap<int, int64_t> mRequestTimestamps;
    QMap<int, QVariant> mValues;
    bool mValueIsConstant;
};

class LocalFileFolderAttributes : public FileFolderAttributes
{
    Q_OBJECT

public:
    LocalFileFolderAttributes(const QString& path, QObject* parent);
    ~LocalFileFolderAttributes() override = default;

    void requestSize(QObject* caller, std::function<void(qint64)> func) override;
    void requestModifiedTime(QObject* caller, std::function<void(const QDateTime&)> func) override;
    void requestCreatedTime(QObject* caller, std::function<void(const QDateTime&)> func) override;
    void requestCRC(QObject* caller, std::function<void(const QString&)> func) override;

    void setPath(const QString &newPath);

private slots:
    void onModifiedTimeCalculated();
    void onSizeCalculated();
    
private:
    bool attributeNeedsUpdate(QObject* caller, int type) override;
    QDateTime calculateModifiedTime();
    qint64 calculateSize();

    QFutureWatcher<qint64> mFolderSizeFuture;
    QFutureWatcher<QDateTime> mModifiedTimeWatcher;
    QString mPath;
    bool mDirectoryIsEmpty;
};

class RemoteFileFolderAttributes : public FileFolderAttributes
{
    Q_OBJECT

public:
    RemoteFileFolderAttributes(const QString& filePath, QObject *parent, bool waitForAttributes);
    RemoteFileFolderAttributes(mega::MegaHandle handle, QObject *parent, bool waitForAttributes);

    ~RemoteFileFolderAttributes() override;

    void initAllAttributes() override;

    void requestSize(QObject* caller, std::function<void(qint64)> func) override;
    void requestFileCount(QObject* caller, std::function<void (int)> func);
    void requestModifiedTime(QObject* caller,std::function<void(const QDateTime&)> func) override;
    void requestCreatedTime(QObject* caller,std::function<void(const QDateTime&)> func) override;
    void requestCRC(QObject* caller,std::function<void(const QString&)> func) override;

    bool isCurrentUser() const;
    void requestUser(QObject* caller, std::function<void(QString)> func);
    void requestVersions(QObject*caller, std::function<void(int)> func);

    int versionCount();
    int fileCount();

    void setHandle(mega::MegaHandle newHandle);

private:
    bool attributeNeedsUpdate(QObject* caller, int type) override;

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
        USER = AttributeTypes::REMOTE_ATTRIBUTES,
        FILE_COUNT,
        VERSIONS
    };

    QString mUserEmail;
    mega::MegaHandle mOwner = mega::INVALID_HANDLE;
    bool mIsCurrentUser = false;
    std::shared_ptr<const UserAttributes::FullName> mUserFullName;

    bool mWaitForAttributes;

    QEventLoop mEventLoop;
};

#endif // FILEFOLDERATTRIBUTES_H
