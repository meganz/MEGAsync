#ifndef EXPORTPROCESSOR_H
#define EXPORTPROCESSOR_H

#include <QStringList>
#include <sdk/megaapi.h>
#include <sdk/qt/QTMegaRequestListener.h>

class ExportProcessor : public QTMegaRequestListener
{
    Q_OBJECT
public:
    explicit ExportProcessor(MegaApi *megaApi, QStringList fileList);

    void requestLinks();
    QStringList getValidLinks();

signals:
    void onRequestLinksFinished();

public slots:
    virtual void QTonRequestFinish(MegaApi* api, MegaRequest *request, MegaError* e);

protected:
    MegaApi *megaApi;
    QStringList fileList;
    QStringList publicLinks;
    QStringList validPublicLinks;
    int currentIndex;
    int remainingNodes;
    int importSuccess;
    int importFailed;
};

#endif // EXPORTPROCESSOR_H
