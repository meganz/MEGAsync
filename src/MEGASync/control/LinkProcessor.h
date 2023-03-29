#ifndef LINKPROCESSOR_H
#define LINKPROCESSOR_H

#include <QObject>
#include <QStringList>
#include <memory>

#include "megaapi.h"
#include "QTMegaRequestListener.h"

class LinkProcessor: public QObject, public mega::MegaRequestListener
{
    Q_OBJECT

public:
    LinkProcessor(QStringList linkList, mega::MegaApi *megaApi, mega::MegaApi *megaApiFolders);
    virtual ~LinkProcessor();

    QString getLink(int id);
    bool isSelected(int id);
    int getError(int id);
    std::shared_ptr<mega::MegaNode> getNode(int id);
    int size() const;

    void requestLinkInfo();
    void importLinks(QString nodePath);
    void importLinks(mega::MegaNode *node);
    mega::MegaHandle getImportParentFolder();

    void downloadLinks(const QString& localPath);
    void setSelected(int linkId, bool selected);

    int numSuccessfullImports();
    int numFailedImports();
    int getCurrentIndex();

    bool atLeastOneLinkValidAndSelected() const;

protected:
    mega::MegaApi *megaApi;
    mega::MegaApi *megaApiFolders;
    QStringList linkList;
    QList<bool> linkSelected;
    QList<std::shared_ptr<mega::MegaNode>> mLinkNode;
    QList<int> linkError;
    int currentIndex;
    int remainingNodes;
    int importSuccess;
    int importFailed;
    mega::MegaHandle importParentFolder;
    mega::QTMegaRequestListener *delegateListener;

signals:
    void onLinkInfoAvailable(int i);
    void onLinkInfoRequestFinish();
    void onLinkImportFinish();
    void dupplicateDownload(QString localPath, QString name, mega::MegaHandle handle, QString nodeKey);

public slots:
    virtual void onRequestFinish(mega::MegaApi* api, mega::MegaRequest *request, mega::MegaError* e);

private:
    void startDownload(mega::MegaNode* linkNode, const QString& localPath);
};

#endif // LINKPROCESSOR_H
