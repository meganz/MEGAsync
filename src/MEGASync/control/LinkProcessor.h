#ifndef LINKPROCESSOR_H
#define LINKPROCESSOR_H

#include <QObject>
#include <QStringList>
#include <QPointer>
#include <memory>
#include "megaapi.h"
#include "QTMegaTransferListener.h"
#include <QSharedPointer>
#include <QQueue>
#include <QList>
#include "LinkObject.h"
#include "SetTypes.h"

namespace mega
{
    class QTMegaRequestListener;
}

enum class LinkTransferType { UNKNOWN, DOWNLOAD, IMPORT };

struct LinkTransfer
{
    LinkTransfer(std::shared_ptr<LinkObject> linkObject, LinkTransferType transferType)
        : linkObject(linkObject)
        , transferType(transferType)
    {}

    std::shared_ptr<LinkObject> linkObject = nullptr;
    LinkTransferType transferType = LinkTransferType::UNKNOWN;
};

class LinkProcessor: public QObject, public mega::MegaTransferListener
{
    Q_OBJECT

public:
    LinkProcessor(mega::MegaApi* megaApi, mega::MegaApi* megaApiFolders);
    LinkProcessor(const QStringList& linkList, mega::MegaApi* megaApi, mega::MegaApi* megaApiFolders);
    virtual ~LinkProcessor();

    void resetAndSetLinkList(const QStringList& linkList);
    QString getLink(int index) const;
    bool isSelected(int index) const;
    MegaNodeSPtr getNode(int index) const;
    void requestLinkInfo();
    void importLinks(const QString& nodePath);
    mega::MegaHandle getImportParentFolder();
    void downloadLinks(const QString& localPath);

    void onRequestFinish(mega::MegaRequest *request, mega::MegaError* e);

signals:
    void requestFetchSetFromLink(const QString& link);
    void requestDownloadSet(const AlbumCollection& set,
                            const QString& downloadPath,
                            const QList<mega::MegaHandle>& elementHandleList);
    void requestImportSet(const AlbumCollection& set,
                          const SetImportParams& sip,
                          const QList<mega::MegaHandle>& elementHandleList);
    void onLinkInfoRequestFinish();
    void onLinkImportFinish();
    void onLinkInfoAvailable(int index,
                             const QString& name,
                             int status,
                             long long size,
                             bool isFolder);

public slots:
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
    void onLinkSelected(int index, bool selected);
    void refreshLinkInfo();

private:
    template <typename Container>
    inline bool isValidIndex(const Container& container, int index) const
    {
        return (index >= 0 && index < container.size());
    }

    void onTransferFinish(mega::MegaApi* api, mega::MegaTransfer *transfer, mega::MegaError* error) override;

    // Download
    void setDownloadPaths(const QString& downloadPath);
    void startDownload(MegaNodeSPtr linkNode, const QString& localPath);

    // Import
    void setImportParentNode(MegaNodeSPtr importParentNode);
    bool copyNode(MegaNodeSPtr linkNode, MegaNodeSPtr importParentNode);

    inline bool isLinkObjectValid(int index) const;
    void sendLinkInfoAvailableSignal(int index);
    void continueOrFinishLinkInfoReq();
    void createInvalidLinkObject(int index, int error);

    void addTransfersAndStartIfNotStartedYet(LinkTransferType transferType);
    void processNextTransfer();

private:
    mega::MegaApi* mMegaApi;
    mega::MegaApi* mMegaApiFolders;
    QStringList mLinkList;
    QList<std::shared_ptr<LinkObject>> mLinkObjects;
    mega::MegaHandle mImportParentFolder;
    std::shared_ptr<mega::QTMegaRequestListener> mDelegateListener;
    std::shared_ptr<mega::QTMegaTransferListener> mDelegateTransferListener;
    uint32_t mRequestCounter;
    int mCurrentIndex;
    QQueue<LinkTransfer> mTransferQueue;
};

#endif // LINKPROCESSOR_H
