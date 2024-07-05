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
#include "SetTypes.h"

enum class SetManagerState
{
    INIT = 0,
    WAIT_FOR_PREVIEW_SET_TO_GET_DATA = 1,
    WAIT_FOR_PREVIEW_SET_TO_DOWNLOAD_FROM_LINK = 2,
    WAIT_FOR_PREVIEW_SET_TO_DOWNLOAD_COLLECTION = 3,
    WAIT_FOR_PREVIEW_SET_TO_IMPORT_COLLECTION = 4
};

enum class ActionType
{
    UNDEFINED = 0,
    REQUEST_FETCH_SET_FROM_LINK = 1,
    REQUEST_DOWNLOAD_SET_FROM_LINK = 2,
    REQUEST_DOWNLOAD_SET = 3,
    REQUEST_IMPORT_SET = 4,
    HANDLE_SET_IN_PREVIEW_MODE = 5,
    HANDLE_ELEMENT_IN_PREVIEW_MODE = 6
};

struct ActionParams
{
    ActionType type = ActionType::UNDEFINED;
    QString link;
    AlbumCollection set;
    QList<mega::MegaHandle> elementHandleList;
    QString downloadPath;
    MegaNodeSPtr importParentNode;
};

class SetManager: public QObject, public mega::MegaTransferListener, public AsyncHandler<bool>
{
    Q_OBJECT

public:
    SetManager(mega::MegaApi* megaApi, mega::MegaApi* megaApiFolders);
    virtual ~SetManager();

signals:
    void onFetchSetFromLink(const AlbumCollection& collection);
    void onSetDownloadFinished(const QString& setName,
                               const QStringList& succeededDownloadedElements,
                               const QStringList& failedDownloadedElements,
                               const QString& destinationPath);
    void onSetImportFinished(const QString& setName,
                             const QStringList& succeededImportElements,
                             const QStringList& failedImportElements,
                             const QStringList& alreadyExistingImportElements,
                             const SetImportParams& sip);

public slots:
    void requestFetchSetFromLink(const QString& link);
    void requestDownloadSetFromLink(const QString& link,
                                    const QString& downloadPath,
                                    const QList<mega::MegaHandle>& elementHandleList);
    void requestDownloadSet(const AlbumCollection& set,
                            const QString& downloadPath,
                            const QList<mega::MegaHandle>& elementHandleList);
    void requestImportSet(const AlbumCollection& set,
                          const SetImportParams& sip,
                          const QList<mega::MegaHandle>& elementHandleList);

private:
    void handleTriggerAction(bool&) override;
    void onRequestFinish(mega::MegaRequest* request, mega::MegaError* error);
    void onTransferFinish(mega::MegaApi* api, mega::MegaTransfer* transfer, mega::MegaError* error) override;

    // State Machine
    void handleStates();
    void handleStateINIT(const ActionParams& action);
    void handleStateWAIT_FOR_PREVIEW_SET_TO_GET_DATA(const ActionParams& action);
    void handleStateWAIT_FOR_PREVIEW_SET_TO_DOWNLOAD_FROM_LINK(const ActionParams& action);
    void handleStateWAIT_FOR_PREVIEW_SET_TO_DOWNLOAD_COLLECTION(const ActionParams& action);
    void handleStateWAIT_FOR_PREVIEW_SET_TO_IMPORT_COLLECTION(const ActionParams& action);

    void handleFetchPublicSetResponse(mega::MegaRequest* request, mega::MegaError* error);
    bool handleFetchPublicSetResponseToGetData();
    bool handleFetchPublicSetResponseToDownloadFromLink();
    bool handleFetchPublicSetResponseToDownloadCollection();
    bool handleFetchPublicSetResponseToImportCollection();
    void handleGetPreviewElementNodeResponse(mega::MegaRequest* request, mega::MegaError* error);
    void handleCreateFolderResponse(mega::MegaRequest* request, mega::MegaError* error);
    void handleCopyNodeResponse(mega::MegaRequest* request, mega::MegaError* error);

    bool createDirectory(const QString& path);
    bool getPreviewSetData();
    bool getPreviewElementNodes();
    AlbumCollection filterSet(const AlbumCollection& srcSet, const QList<mega::MegaHandle>& elementHandleList);
    void reset();
    void resetAndHandleStates();
    void startDownload(mega::MegaNode* linkNode, const QString& localPath);
    bool copyNode(MegaNodeSPtr linkNode, MegaNodeSPtr importParentNode);
    void checkandHandleFinishedImport();

private:
    mega::MegaApi* mMegaApi;
    mega::MegaApi* mMegaApiFolders;
    std::shared_ptr<mega::QTMegaRequestListener> mDelegateListener;
    std::shared_ptr<mega::QTMegaTransferListener> mDelegateTransferListener;

    AlbumCollection mCurrentSet;
    QList<mega::MegaHandle> mCurrentElementHandleList;
    QString mCurrentDownloadPath;
    MegaNodeSPtr mCurrentImportParentNode;

    QStringList mFailedDownloadedElements;
    QStringList mSucceededDownloadedElements;
    QStringList mFailedImportElements;
    QStringList mSucceededImportElements;
    QStringList mAlreadyExistingImportElements;

    // State machine
    QMutex mSetManagerStateMutex;
    SetManagerState mSetManagerState;
    ProtectedQueue<ActionParams> mInternalActionQueue;  // Internal actions
    ProtectedQueue<ActionParams> mRequestQueue; // Requests from users
};

#endif // SET_MANAGER_H
