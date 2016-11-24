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
    explicit ExportProcessor(mega::MegaApi *megaApi, QList<mega::MegaHandle> handleList);
    virtual ~ExportProcessor();

    void requestLinks();
    QStringList getValidLinks();

signals:
    void onRequestLinksFinished();

public slots:
    virtual void onRequestFinish(mega::MegaApi* api, mega::MegaRequest *request, mega::MegaError* e);

protected:
    enum {
        MODE_PATHS,
        MODE_HANDLES
    };

    mega::MegaApi *megaApi;
    QStringList fileList;
    QList<mega::MegaHandle> handleList;
    QStringList publicLinks;
    QStringList validPublicLinks;
    int currentIndex;
    int remainingNodes;
    int importSuccess;
    int importFailed;
    int mode;
    mega::QTMegaRequestListener *delegateListener;
};

#endif // EXPORTPROCESSOR_H
