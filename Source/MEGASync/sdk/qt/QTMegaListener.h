#ifndef QTMEGALISTENER_H
#define QTMEGALISTENER_H

#include<QObject>
#include "megaapi.h"

class QTMegaListener : public QObject, public MegaListener
{
	Q_OBJECT

public:
    explicit QTMegaListener(MegaApi *megaApi, MegaListener *parent=NULL);
    virtual ~QTMegaListener();

	virtual void onRequestStart(MegaApi* api, MegaRequest *request);
	virtual void onRequestFinish(MegaApi* api, MegaRequest *request, MegaError* e);
    virtual void onRequestUpdate(MegaApi* api, MegaRequest *request);
	virtual void onRequestTemporaryError(MegaApi *api, MegaRequest *request, MegaError* e);
	virtual void onTransferStart(MegaApi *api, MegaTransfer *transfer);
	virtual void onTransferFinish(MegaApi* api, MegaTransfer *transfer, MegaError* e);
	virtual void onTransferUpdate(MegaApi *api, MegaTransfer *transfer);
	virtual void onTransferTemporaryError(MegaApi *api, MegaTransfer *transfer, MegaError* e);
	virtual void onUsersUpdate(MegaApi* api, UserList *users);
	virtual void onNodesUpdate(MegaApi* api, NodeList *nodes);
	virtual void onReloadNeeded(MegaApi* api);
    virtual void onSyncStateChanged(MegaApi* api);

signals:
    void QTonRequestStartSignal(MegaApi* api, MegaRequest *request);
    void QTonRequestFinishSignal(MegaApi* api, MegaRequest *request, MegaError* e);
    void QTonRequestUpdateSignal(MegaApi* api, MegaRequest *request);
    void QTonRequestTemporaryErrorSignal(MegaApi *api, MegaRequest *request, MegaError* e);
    void QTonTransferStartSignal(MegaApi *api, MegaTransfer *transfer);
    void QTonTransferFinishSignal(MegaApi* api, MegaTransfer *transfer, MegaError* e);
    void QTonTransferUpdateSignal(MegaApi *api, MegaTransfer *transfer);
    void QTonTransferTemporaryErrorSignal(MegaApi *api, MegaTransfer *transfer, MegaError* e);
    void QTonUsersUpdateSignal(MegaApi* api, UserList *users);
    void QTonNodesUpdateSignal(MegaApi* api, NodeList *nodes);
    void QTonReloadNeededSignal(MegaApi* api);
    void QTonSyncStateChangedSignal(MegaApi* api);

public slots:
	virtual void QTonRequestStart(MegaApi* api, MegaRequest *request);
	virtual void QTonRequestFinish(MegaApi* api, MegaRequest *request, MegaError* e);
    virtual void QTonRequestUpdate(MegaApi* api, MegaRequest *request);
	virtual void QTonRequestTemporaryError(MegaApi *api, MegaRequest *request, MegaError* e);
	virtual void QTonTransferStart(MegaApi *api, MegaTransfer *transfer);
	virtual void QTonTransferFinish(MegaApi* api, MegaTransfer *transfer, MegaError* e);
	virtual void QTonTransferUpdate(MegaApi *api, MegaTransfer *transfer);
	virtual void QTonTransferTemporaryError(MegaApi *api, MegaTransfer *transfer, MegaError* e);
	virtual void QTonUsersUpdate(MegaApi* api, UserList *users);
	virtual void QTonNodesUpdate(MegaApi* api, NodeList *nodes);
	virtual void QTonReloadNeeded(MegaApi* api);
    virtual void QTonSyncStateChanged(MegaApi* api);

protected:
    MegaApi *megaApi;
	MegaListener *listener;
};

#endif // QTMEGALISTENER_H
