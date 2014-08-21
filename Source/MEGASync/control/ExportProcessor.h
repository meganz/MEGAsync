#ifndef EXPORTPROCESSOR_H
#define EXPORTPROCESSOR_H

#include <QStringList>
#include <megaapi.h>
#include <QTMegaRequestListener.h>

class ExportProcessor : public mega::QTMegaRequestListener
{
    Q_OBJECT
public:
    explicit ExportProcessor(mega::MegaApi *megaApi, QStringList fileList);

    void requestLinks();
    QStringList getValidLinks();

signals:
    void onRequestLinksFinished();

public slots:
    virtual void QTonRequestFinish(mega::MegaApi* api, mega::MegaRequest *request, mega::MegaError* e);

protected:
    mega::MegaApi *megaApi;
    QStringList fileList;
    QStringList publicLinks;
    QStringList validPublicLinks;
    int currentIndex;
    int remainingNodes;
    int importSuccess;
    int importFailed;
};

#endif // EXPORTPROCESSOR_H
