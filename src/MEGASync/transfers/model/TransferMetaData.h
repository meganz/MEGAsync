#ifndef TRANSFERMETADATA_H
#define TRANSFERMETADATA_H

#include <memory>
#include <QVariant>
#include <QPair>
#include <QPointer>

#include <Preferences.h>

namespace mega
{
    class MegaTransfer;
    class MegaError;
    class MegaNode;
}

class MegaNotification;

class TransferMetaData
{
public:
    TransferMetaData(int8_t direction, unsigned long long id);
    TransferMetaData();
    virtual ~TransferMetaData() = default;

    virtual QVariant getData() const {return QVariant();}
    bool remove();

    virtual void updateOnTransferFinish(mega::MegaTransfer*, mega::MegaError *);

    bool isSingleTransfer() const;

    bool readyForNotification() const;
    int getTotalTransfers() const;
    int getPendingTransfers() const;
    int getTotalFiles() const;
    int getTotalFolders() const;
    int getTransfersFileOK() const;
    int getTransfersFolderOK() const;
    int getTransfersOK() const;
    int getTransfersFailed() const;
    int getTransfersCancelled() const;

    void addFile();
    void addFolder();

    void removePendingFile();
    void removeFailingItem(const mega::MegaHandle& handle);
    void checkAndSendNotification();

    bool isDownload() const;
    bool isUpload() const;

    bool isFolderTransfer() const;
    bool isFileTransfer() const;

    bool getCreateRootFolder() const;
    void setCreateRootFolder(bool newCreateRootFolder);

    void blockNotification();

    unsigned long long getAppId() const;

    void setPendingTransfers(int newPendingTransfers);

    void setForSpareTransfers(bool newForSpareTransfers);

    void setNotification(MegaNotification* newNotification);

protected:
    int mPendingTransfers;
    int mTotalFiles;
    int mTotalFolders;
    int mTransfersFileOK;
    int mTransfersFolderOK;
    QList<mega::MegaHandle> mTransfersFailed;
    int mTransfersCancelled;
    int8_t mTransferDirection;
    bool mCreateRootFolder;
    bool mNotificationBlocked;
    bool mForSpareTransfers;
    unsigned long long mAppId;

private:
    QPointer<MegaNotification> mNotification;
    QMetaObject::Connection mNotificationDestroyed;

};

Q_DECLARE_METATYPE(std::shared_ptr<TransferMetaData>)

class DownloadTransferMetaData : public TransferMetaData
{
public:
    DownloadTransferMetaData(unsigned long long appId, const QString& path);
     ~DownloadTransferMetaData() = default;

    void update(mega::MegaNode* node);
    void updateForeignDir();
    QVariant getData() const override;

    static QString getDestinationNodePathByData(const std::shared_ptr<TransferMetaData>& data);
    void updateOnTransferFinish(mega::MegaTransfer*transfer, mega::MegaError *e) override;

    const QStringList &getLocalPaths() const;
    const QString &getLocalTargetPath() const;

private:
    //DOWNLOAD 1 node (file or folder) -> file/folder local path
    //DOWNLOAD some nodes -> destination local path
    QStringList mLocalPaths;
    QString mLocalTargetPath;
};

Q_DECLARE_METATYPE(std::shared_ptr<DownloadTransferMetaData>)

class UploadTransferMetaData : public TransferMetaData
{
public:
    UploadTransferMetaData(unsigned long long appId, const mega::MegaHandle handle);
     ~UploadTransferMetaData() = default;

    void update(const QString &nodePath);
    void update(bool isDir);
    QVariant getData() const override;
    mega::MegaHandle getNodeTarget() const;

    void updateOnTransferFinish(mega::MegaTransfer* transfer, mega::MegaError* e) override;

    static std::shared_ptr<mega::MegaNode> getDestinationNodeByData(const std::shared_ptr<TransferMetaData>& data);
    static QList<std::shared_ptr<mega::MegaNode>> getNodesByData(const std::shared_ptr<TransferMetaData>& data);

private:
    //UPLOAD 1 node (file or folder) -> remote node handle
    //UPLOAD some nodes -> remote destination node handle
    QList<mega::MegaHandle> mNodeHandles;
    mega::MegaHandle mNodeTargetHandle;
};

Q_DECLARE_METATYPE(std::shared_ptr<UploadTransferMetaData>)

class TransferMetaDataContainer
{
public:
    template <typename TYPE = TransferMetaData>
    static std::shared_ptr<TYPE> getAppData(unsigned long long appId)
    {
        auto data = transferAppData.value(appId);
        return std::dynamic_pointer_cast<TYPE>(data);
    }

    static bool addAppData(unsigned long long appId, std::shared_ptr<TransferMetaData> data);
    static bool removeAppData(unsigned long long appId);
    static void updateOnTransferFinish(unsigned long long appId, mega::MegaTransfer* transfer, mega::MegaError* e);

    template <typename TYPE, typename... A>
    static std::shared_ptr<TYPE> createTransferMetaData(A &&...args)
    {
        unsigned long long transferId = Preferences::instance()->transferIdentifier();
        std::shared_ptr<TYPE> transferData =  std::make_shared<TYPE>(transferId, std::forward<A>(args)...);
        addAppData(transferId, transferData);
        return transferData;
    }

    template <typename TYPE, typename... A>
    static std::shared_ptr<TYPE> createTransferMetaDataWithTransferId(unsigned long long transferId, A &&...args)
    {
        std::shared_ptr<TYPE> transferData =  std::make_shared<TYPE>(transferId, std::forward<A>(args)...);
        transferData->setForSpareTransfers(true);
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
};

#endif // TRANSFERMETADATA_H
