#ifndef EXPORTPROCESSOR_H
#define EXPORTPROCESSOR_H

#include <QStringList>
#include <megaapi.h>
#include <QTMegaRequestListener.h>

class ExportProcessor :  public QObject, public mega::MegaRequestListener
{
    Q_OBJECT
public:
    explicit ExportProcessor(mega::MegaApi *megaApi, QStringList fileList);
    virtual ~ExportProcessor();

    void requestLinks();
    QStringList getValidLinks();

signals:
    void onRequestLinksFinished();

public slots:
    virtual void onRequestFinish(mega::MegaApi* api, mega::MegaRequest *request, mega::MegaError* e);

protected:
    mega::MegaApi *megaApi;
    QStringList fileList;
    QStringList publicLinks;
    QStringList validPublicLinks;
    int currentIndex;
    int remainingNodes;
    int importSuccess;
    int importFailed;
    mega::QTMegaRequestListener *delegateListener;
};

#endif // EXPORTPROCESSOR_H
