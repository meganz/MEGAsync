#ifndef LINKPROCESSOR_H
#define LINKPROCESSOR_H

#include <QObject>
#include <QStringList>
#include "megaapi.h"
#include "QTMegaRequestListener.h"

class LinkProcessor: public QObject, public mega::MegaRequestListener
{
    Q_OBJECT

public:
    LinkProcessor(mega::MegaApi *megaApi, mega::MegaApi *megaApiGuest, QStringList linkList);
    virtual ~LinkProcessor();

    QString getLink(int id);
    bool isSelected(int id);
    int getError(int id);
    mega::MegaNode *getNode(int id);
    int size();

    void requestLinkInfo();
    void importLinks(QString nodePath);
    void importLinks(mega::MegaNode *node);
    mega::MegaHandle getImportParentFolder();

    void downloadLinks(QString localPath);
    void setSelected(int linkId, bool selected);

    int numSuccessfullImports();
    int numFailedImports();
    int getCurrentIndex();

protected:
    mega::MegaApi *megaApi;
    mega::MegaApi *megaApiGuest;
    QStringList linkList;
    QList<bool> linkSelected;
    QList<mega::MegaNode *> linkNode;
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
    void onDupplicateLink(QString link, QString name, mega::MegaHandle handle);
    void dupplicateDownload(QString localPath, QString name, mega::MegaHandle handle);

public slots:
    virtual void onRequestFinish(mega::MegaApi* api, mega::MegaRequest *request, mega::MegaError* e);
};

#endif // LINKPROCESSOR_H
