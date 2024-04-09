#ifndef SET_MANAGER_H
#define SET_MANAGER_H

#include <QObject>
#include <QList>
#include <QMutex>
#include <memory>
#include "megaapi.h"
#include "QTMegaRequestListener.h"
#include "QTMegaTransferListener.h"
#include "AsyncHandler.h"

using MegaNodeSPtr = std::shared_ptr<mega::MegaNode>;

struct AlbumCollection
{
    QString link = QString::fromUtf8("");
    QString name = QString::fromUtf8("");
    QList<mega::MegaHandle> elementHandleList = {};
    QList<MegaNodeSPtr> nodeList = {};

    // Default constructor
    AlbumCollection() = default;
    ~AlbumCollection()
    {
        reset();
    }

    void reset()
    {
        link = QString::fromUtf8("");
        name = QString::fromUtf8("");
        elementHandleList.clear();
        nodeList.clear();
    }

    bool isComplete() const
    {
        return (!link.isEmpty() &&
                !name.isEmpty() &&
                !elementHandleList.isEmpty() &&
                (elementHandleList.size() == nodeList.size()));
    }
};

Q_DECLARE_METATYPE(AlbumCollection)

enum class SetManagerState
{
    INIT = 0,
    WAIT_FOR_PREVIEW_SET_TO_GET_DATA = 1,
    WAIT_FOR_PREVIEW_SET_TO_DOWNLOAD_FROM_LINK = 2,
    WAIT_FOR_PREVIEW_SET_TO_DOWNLOAD_COLLECTION = 3
};

enum class CallbackType
{
    UNDEFINED = 0,
    REQUEST_FETCH_SET_FROM_LINK = 1,
    REQUEST_DOWNLOAD_SET_FROM_LINK = 2,
    REQUEST_DOWNLOAD_SET = 3
};

struct CallbackAction
{
    CallbackType type = CallbackType::UNDEFINED;
    QString link;
    AlbumCollection set;
     QList<mega::MegaHandle> elementHandleList;
    QString downloadPath;
};

class SetManager: public QObject, public mega::MegaRequestListener, public mega::MegaTransferListener, public AsyncHandler<struct CallbackAction>
{
    Q_OBJECT

public:
    SetManager(mega::MegaApi* megaApi, mega::MegaApi* megaApiFolders);
    virtual ~SetManager();

signals:
    void onFetchSetFromLink(const AlbumCollection& collection);
    void onSetDownloaded(const QString& setName, int nrDownloadedElements);

public slots:
    void requestFetchSetFromLink(const QString& link);
    void requestDownloadSetFromLink(const QString& link,
                                    const QString& downloadPath,
                                    const QList<mega::MegaHandle>& elementHandleList);
    void requestDownloadSet(const AlbumCollection& set,
                            const QString& downloadPath,
                            const QList<mega::MegaHandle>& elementHandleList);

private:
    void handleTriggerAction(struct CallbackAction& action) override;
    void onRequestFinish(mega::MegaApi* api, mega::MegaRequest* request, mega::MegaError* error) override;
    void onTransferFinish(mega::MegaApi* api, mega::MegaTransfer* transfer, mega::MegaError* error) override;

    void handleFetchPublicSetResponse(mega::MegaRequest* request, mega::MegaError* error);
    bool handleFetchPublicSetResponseToGetData();
    bool handleFetchPublicSetResponseToDownloadFromLink();
    bool handleFetchPublicSetResponseToDownloadCollection();
    void handleGetPreviewElementNodeResponse(mega::MegaRequest* request, mega::MegaError* error);

    bool createDirectory(const QString& path);
    bool getPreviewSetData();
    bool getPreviewElementNodes();
    AlbumCollection filterSet(const AlbumCollection& srcSet, const QList<mega::MegaHandle>& elementHandleList);
    void reset();
    void startDownload(mega::MegaNode* linkNode, const QString& localPath);

private:
    mega::MegaApi* mMegaApi;
    mega::MegaApi* mMegaApiFolders;
    std::shared_ptr<mega::QTMegaRequestListener> mDelegateListener;
    std::shared_ptr<mega::QTMegaTransferListener> mDelegateTransferListener;

    // State machine
    QMutex mSetManagerStateMutex;
    SetManagerState mSetManagerState;

    AlbumCollection mCurrentSet;
    QList<mega::MegaHandle> mCurrentElementHandleList;
    QString mCurrentDownloadPath;

    int mNrDownloadedElements;
};

#endif // SET_MANAGER_H
