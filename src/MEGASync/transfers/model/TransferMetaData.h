#ifndef TRANSFERMETADATA_H
#define TRANSFERMETADATA_H

#include <memory>
#include <QVariant>
#include <QPair>
#include <QPointer>
#include <QMutex>

#include <Preferences.h>
#include "TransferItem.h"

namespace mega
{
    class MegaTransfer;
    class MegaError;
    class MegaNode;
}

class MegaNotification;
class TransferItem;

struct TransferMetaDataItemId
{
    TransferMetaDataItemId(int utag, mega::MegaHandle uhandle):tag(utag), handle(uhandle){}
    TransferMetaDataItemId(int utag, mega::MegaHandle uhandle, const QString& uname, const QString& upath):tag(utag), handle(uhandle), name(uname), path(upath){}
    TransferMetaDataItemId():tag(0), handle(mega::INVALID_HANDLE){}

    int tag;
    mega::MegaHandle handle;
    QString name;
    QString path;

    bool operator==(const TransferMetaDataItemId &data) const;
    bool operator<(const TransferMetaDataItemId &data) const;
    bool isValid() const
    {
        return tag > 0 || handle != mega::INVALID_HANDLE;
    }
};

struct TransferMetaDataItem
{
    TransferMetaDataItem(const TransferMetaDataItemId& uid)
        :id(uid), state(TransferData::TRANSFER_ACTIVE), failedTransfer(nullptr){}

    ~TransferMetaDataItem()
    {
    }

    TransferMetaDataItemId id;
    TransferMetaDataItemId topLevelFolderId;
    TransferMetaDataItemId folderId;
    TransferData::TransferState state;
    std::shared_ptr<mega::MegaTransfer> failedTransfer;

    virtual TransferData::TransferState getState(){return state;}
};

template <class Type>
struct TransferMetaDataItemsByState
{ 
    int size() const {return pendingTransfers.size() + completedTransfers.size() + failedTransfers.size() + cancelledTransfers.size();}
    QList<TransferMetaDataItemId> ids() const
    {
        QList<TransferMetaDataItemId> ids;
        ids << pendingTransfers.keys() << completedTransfers.keys() << failedTransfers.keys() << cancelledTransfers.keys();
        return ids;
    }

    QMap<TransferMetaDataItemId, std::shared_ptr<Type>> pendingTransfers;
    QMap<TransferMetaDataItemId, std::shared_ptr<Type>> completedTransfers;
    QMultiMap<mega::MegaHandle, std::shared_ptr<Type>> completedTransfersByFolderHandle;
    QMap<TransferMetaDataItemId, std::shared_ptr<Type>> failedTransfers;
    QMap<TransferMetaDataItemId, std::shared_ptr<Type>> nonExistFailedTransfers;
    QMap<TransferMetaDataItemId, std::shared_ptr<Type>> cancelledTransfers;

    TransferMetaDataItemId getFirstTransferIdByState(TransferData::TransferState state) const
    {
        switch(state)
        {
            case TransferData::TRANSFER_COMPLETED:
            {
                if(!completedTransfers.isEmpty())
                {
                    return completedTransfers.first()->id;
                }
                break;
            }
            case TransferData::TRANSFER_CANCELLED:
            {
                if(!cancelledTransfers.isEmpty())
                {
                    return cancelledTransfers.first()->id;
                }
                break;
            }
            case TransferData::TRANSFER_FAILED:
            {
                if(!failedTransfers.isEmpty())
                {
                    return failedTransfers.first()->id;
                }
                if(!nonExistFailedTransfers.isEmpty())
                {
                    return nonExistFailedTransfers.first()->id;
                }
                break;
            }
            default:
            {
                if(!pendingTransfers.isEmpty())
                {
                    return pendingTransfers.first()->id;
                }
                break;
            }
        }

        return TransferMetaDataItemId();
    }
    QList<TransferMetaDataItemId> getTransferIdsByState(TransferData::TransferState state) const
    {
        QList<TransferMetaDataItemId> ids;

        switch(state)
        {
            case TransferData::TRANSFER_COMPLETED:
            {
                if(!completedTransfers.isEmpty())
                {
                    foreach(auto& item, completedTransfers.keys())
                    {
                        ids.append(item);
                    }
                }
                break;
            }
            case TransferData::TRANSFER_CANCELLED:
            {
                if(!cancelledTransfers.isEmpty())
                {
                    foreach(auto& item, cancelledTransfers.keys())
                    {
                        ids.append(item);
                    }
                }
                break;
            }
            case TransferData::TRANSFER_FAILED:
            {
                if(!failedTransfers.isEmpty())
                {
                    foreach(auto& item, failedTransfers.keys())
                    {
                        ids.append(item);
                    }
                }
                break;
            }
            default:
            {
                if(!pendingTransfers.isEmpty())
                {
                    foreach(auto& item, pendingTransfers.keys())
                    {
                        ids.append(item);
                    }
                }
                break;
            }
        }

        return ids;
    }

    bool hasChanged() const
    {
        return mHasChanged;
    }

    void setHasChanged(bool newHasChanged)
    {
        mHasChanged = newHasChanged;
    }

private:
    bool mHasChanged = false;

    friend class TransferMetaData;
    void insertItem(TransferData::TransferState state, const std::shared_ptr<Type>& item)
    {
        removeItem(item);

        item->state = state;
        switch(state)
        {
            case TransferData::TRANSFER_COMPLETED:
            {
                completedTransfers.insert(item->id, item);
                completedTransfersByFolderHandle.insert(item->folderId.handle, item);
                break;
            }
            case TransferData::TRANSFER_CANCELLED:
            {
                cancelledTransfers.insert(item->id, item);
                break;
            }
            case TransferData::TRANSFER_FAILED:
            {
                failedTransfers.insert(item->id, item);
                break;
            }
            default:
            {
                pendingTransfers.insert(item->id, item);
                break;
            }
        }

        setHasChanged(true);
    }

    void removeItem(const std::shared_ptr<Type>& item)
    {
        switch(item->state)
        {
            case TransferData::TRANSFER_COMPLETED:
            {
                completedTransfers.remove(item->id);
                completedTransfersByFolderHandle.remove(item->folderId.handle, item);
                break;
            }
            case TransferData::TRANSFER_CANCELLED:
            {
                cancelledTransfers.remove(item->id);
                break;
            }
            case TransferData::TRANSFER_FAILED:
            {
                failedTransfers.remove(item->id);
                break;
            }
            default:
            {
                pendingTransfers.remove(item->id);
                break;
            }
        }
    }
};

struct TransferMetaDataFolderItem : public TransferMetaDataItem
{
    TransferMetaDataFolderItem(const TransferMetaDataItemId& id)
        : TransferMetaDataItem(id){}

    TransferMetaDataItemsByState<TransferMetaDataItem> files;
};

class TransferMetaData
{
public:
    TransferMetaData(int direction, unsigned long long id);
    TransferMetaData();
    virtual ~TransferMetaData() = default;

    virtual bool finish(mega::MegaTransfer*, mega::MegaError *);

    void setInitialPendingTransfers(int newTotalTransfers);
    void setNotification(MegaNotification* newNotification);
    void unlinkNotification();

    unsigned long long getAppId() const;
    void checkAndSendNotification();
    void remove();

    bool isNonExistData() const;
    bool isSingleTransfer() const;
    bool isDownload() const;
    bool isUpload() const;
    bool allHaveFailed() const;
    bool someHaveFailed() const;
    bool isEmpty() const;

    //For files
    int getTotalFiles() const;
    int getFileTransfersOK() const;
    int getFileTransfersFailed() const;
    void getFileTransferFailedTags(QList<std::shared_ptr<TransferMetaDataItem> > &files, QList<TransferMetaDataItemId>& folders) const;
    QList<TransferMetaDataItemId> getFileFailedTagsFromFolderTag(const TransferMetaDataItemId& folderId) const;
    int getFileTransfersCancelled() const;

    TransferMetaDataItemId getFirstTransferIdByState(TransferData::TransferState state) const;
    QList<TransferMetaDataItemId> getTransferIdsByState(TransferData::TransferState state) const;

    void addFile(int tag);
    void addFileFromFolder(int folderTag, int fileTag);

    //Top Level folders
    void topLevelFolderScanningFinished(int filecount);

    //Folder controller
    void processCancelled();

    //For folders
    int getTotalEmptyFolders() const;
    int getEmptyFolderTransfersOK() const;
    int getEmptyFolderTransfersFailed() const;

    virtual bool hasBeenPreviouslyCompleted(mega::MegaTransfer* transfer) const = 0;

    bool isRetriedFolder(const TransferMetaDataItemId& folderId) const;
    bool isRetriedFolder(mega::MegaTransfer *transfer);

    void setCreatedFromOtherSession();

protected:
    friend class TransferMetaDataContainer;
    virtual void start(mega::MegaTransfer* transfer);
    virtual std::shared_ptr<TransferMetaData> createNonExistData() = 0;
    virtual bool isNonExistTransfer(mega::MegaTransfer* transfer) const = 0;

    int mInitialPendingTransfers;
    int mInitialPendingFolderTransfersFromOtherSession;

    TransferMetaDataItemsByState<TransferMetaDataItem> mFiles;
    QMap<TransferMetaDataItemId, std::shared_ptr<TransferMetaDataFolderItem>> mFolders;
    TransferMetaDataItemsByState<TransferMetaDataFolderItem> mEmptyFolders;
    int getEmptyFolders(const QMap<TransferMetaDataItemId, std::shared_ptr<TransferMetaDataFolderItem> > &folders) const;

    int mTransferDirection;
    bool mCreateRootFolder;
    unsigned long long mAppId;
    bool mCreatedFromOtherSession;

private:
    void retryFailingFile(int tag, mega::MegaHandle nodeHandle);
    void retryFileFromFolderFailingItem(int fileTag, int folderTag, int nodeHandle);
    void retryAllPressed();

    void addInitialPendingTransfer();
    void decreaseInitialPendingTransfers(mega::MegaTransfer* transfer);

    QPointer<MegaNotification> mNotification;
    QMetaObject::Connection mNotificationDestroyedConnection;
    unsigned long long mNonExistsFailAppId;
};

Q_DECLARE_METATYPE(std::shared_ptr<TransferMetaData>)

class DownloadTransferMetaData : public TransferMetaData
{
public:
    DownloadTransferMetaData(unsigned long long appId, const QString& path);
     ~DownloadTransferMetaData() = default;

    void start(mega::MegaTransfer* transfer) override;
    bool finish(mega::MegaTransfer*transfer, mega::MegaError *e) override;

    void updateForeignDir(const mega::MegaHandle &);

    static QString getDestinationNodePathByData(const std::shared_ptr<TransferMetaData>& data);

    QStringList getLocalPaths() const;
    const QString &getLocalTargetPath() const;

    bool hasBeenPreviouslyCompleted(mega::MegaTransfer* transfer) const override;

protected:
    std::shared_ptr<TransferMetaData> createNonExistData() override;
    bool isNonExistTransfer(mega::MegaTransfer *transfer) const override;

private:
    QString mLocalTargetPath;
};

Q_DECLARE_METATYPE(std::shared_ptr<DownloadTransferMetaData>)

class UploadTransferMetaData : public TransferMetaData
{
public:
    UploadTransferMetaData(unsigned long long appId, const mega::MegaHandle handle);
     ~UploadTransferMetaData() = default;

    mega::MegaHandle getNodeTarget() const;

    void start(mega::MegaTransfer* transfer) override;
    bool finish(mega::MegaTransfer* transfer, mega::MegaError* e) override;

    static std::shared_ptr<mega::MegaNode> getDestinationNodeByData(const std::shared_ptr<TransferMetaData>& data);
    static QString getDestinationNodePathByData(const std::shared_ptr<TransferMetaData>& data);

    static QList<std::shared_ptr<mega::MegaNode>> getNodesByData(const std::shared_ptr<TransferMetaData>& data);

    bool hasBeenPreviouslyCompleted(mega::MegaTransfer* transfer) const override;

protected:
    std::shared_ptr<TransferMetaData> createNonExistData() override;
    bool isNonExistTransfer(mega::MegaTransfer *transfer) const override;

private:
    mega::MegaHandle mNodeTargetHandle;
};

Q_DECLARE_METATYPE(std::shared_ptr<UploadTransferMetaData>)

class TransferMetaDataContainer
{
public:
    static bool start(mega::MegaTransfer* transfer);
    static void finish(unsigned long long appId, mega::MegaTransfer* transfer, mega::MegaError* e);
    static bool finishFromFolderTransfer(mega::MegaTransfer* transfer, mega::MegaError* e);

    static void retryTransfer(mega::MegaTransfer* transfer, unsigned long long appDataId);
    static void retryAllPressed()
    {
        QMutexLocker lock(&mMutex);
        foreach(auto& appdata, transferAppData)
        {
            appdata->retryAllPressed();
        }
    }

    template <typename TYPE = TransferMetaData>
    static std::shared_ptr<TYPE> getAppData(mega::MegaTransfer* transfer)
    {
        if(transfer->getFolderTransferTag() > 0)
        {
            return getAppDataByFolderTransferTag<TYPE>(transfer->getFolderTransferTag());
        }
        else
        {
            auto appData = appDataToId(transfer->getAppData());
            if(appData.first)
            {
                return getAppData<TYPE>(appData.second);
            }
        }

        return nullptr;
    }

    template <typename TYPE = TransferMetaData>
    static std::shared_ptr<TYPE> getAppData(unsigned long long appId)
    {
        QMutexLocker lock(&mMutex);
        auto data = transferAppData.value(appId);
        return std::dynamic_pointer_cast<TYPE>(data);
    }

    template <typename TYPE = TransferMetaData>
    static std::shared_ptr<TYPE> getAppDataByFolderTransferTag(int tag)
    {
        QMutexLocker lock(&mMutex);
        TransferMetaDataItemId id(tag, mega::INVALID_HANDLE);
        foreach(auto& appdata, transferAppData)
        {
            if(appdata->isRetriedFolder(id))
            {
                return std::dynamic_pointer_cast<TYPE>(appdata);
            }
        }

        return nullptr;
    }

    static bool addAppData(unsigned long long appId, std::shared_ptr<TransferMetaData> data);
    static void removeAppData(unsigned long long appId);

    template <typename TYPE, typename... A>
    static std::shared_ptr<TYPE> createTransferMetaData(A &&...args)
    {
        unsigned long long transferId = Preferences::instance()->transferIdentifier();
        std::shared_ptr<TYPE> transferData =  std::make_shared<TYPE>(transferId, std::forward<A>(args)...);
        addAppData(transferId, transferData);
        return transferData;
    }

    template <typename TYPE, typename... A>
    static std::shared_ptr<TYPE> createTransferMetaDataWithappDataId(unsigned long long transferId, A &&...args)
    {
        std::shared_ptr<TYPE> transferData =  std::make_shared<TYPE>(transferId, std::forward<A>(args)...);
        addAppData(transferId, transferData);
        return transferData;
    }

    static QPair<bool, unsigned long long> appDataToId(const char* appData)
    {
        if (appData)
        {
            char *endptr;
            return qMakePair(true, strtoll(appData, &endptr, 10));
        }

        return qMakePair(false,0);
    }

private:
    static QHash<unsigned long long, std::shared_ptr<TransferMetaData>> transferAppData;
    static QMutex mMutex;
};

#endif // TRANSFERMETADATA_H
