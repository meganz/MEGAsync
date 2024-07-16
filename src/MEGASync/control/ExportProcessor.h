#ifndef EXPORTPROCESSOR_H
#define EXPORTPROCESSOR_H

#include <QObject>
#include <QStringList>
#include <megaapi.h>

class ExportProcessor :  public QObject
{
    Q_OBJECT
public:
    explicit ExportProcessor(mega::MegaApi* megaApi, QStringList fileList);
    explicit ExportProcessor(mega::MegaApi* megaApi, QList<mega::MegaHandle> handleList);

    void requestLinks();
    QStringList getValidLinks();

    void onRequestFinish(mega::MegaRequest *request, mega::MegaError* e);

signals:
    void onRequestLinksFinished();

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

private:
    void init(mega::MegaApi* megaApi, int mode, int size);
};

#endif // EXPORTPROCESSOR_H
