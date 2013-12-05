#ifndef QTMEGALISTENER_H
#define QTMEGALISTENER_H

#include<QObject>
#include <sdk/megaapi.h>

class QTMegaListener : public QObject, public MegaListener
{
	Q_OBJECT

public:
	explicit QTMegaListener(MegaListener *parent=NULL);

	virtual void onRequestStart(MegaApi* api, MegaRequest *request);
	virtual void onRequestFinish(MegaApi* api, MegaRequest *request, MegaError* e);
	virtual void onRequestTemporaryError(MegaApi *api, MegaRequest *request, MegaError* e);
	virtual void onTransferStart(MegaApi *api, MegaTransfer *transfer);
	virtual void onTransferFinish(MegaApi* api, MegaTransfer *transfer, MegaError* e);
	virtual void onTransferUpdate(MegaApi *api, MegaTransfer *transfer);
	virtual void onTransferTemporaryError(MegaApi *api, MegaTransfer *transfer, MegaError* e);
	virtual void onUsersUpdate(MegaApi* api, UserList *users);
	virtual void onNodesUpdate(MegaApi* api, NodeList *nodes);
	virtual void onReloadNeeded(MegaApi* api);

signals:
	virtual void QTonRequestStartSignal(MegaApi* api, MegaRequest *request);
	virtual void QTonRequestFinishSignal(MegaApi* api, MegaRequest *request, MegaError* e);
	virtual void QTonRequestTemporaryErrorSignal(MegaApi *api, MegaRequest *request, MegaError* e);
	virtual void QTonTransferStartSignal(MegaApi *api, MegaTransfer *transfer);
	virtual void QTonTransferFinishSignal(MegaApi* api, MegaTransfer *transfer, MegaError* e);
	virtual void QTonTransferUpdateSignal(MegaApi *api, MegaTransfer *transfer);
	virtual void QTonTransferTemporaryErrorSignal(MegaApi *api, MegaTransfer *transfer, MegaError* e);
	virtual void QTonUsersUpdateSignal(MegaApi* api, UserList *users);
	virtual void QTonNodesUpdateSignal(MegaApi* api, NodeList *nodes);
	virtual void QTonReloadNeededSignal(MegaApi* api);

public slots:
	virtual void QTonRequestStart(MegaApi* api, MegaRequest *request);
	virtual void QTonRequestFinish(MegaApi* api, MegaRequest *request, MegaError* e);
	virtual void QTonRequestTemporaryError(MegaApi *api, MegaRequest *request, MegaError* e);
	virtual void QTonTransferStart(MegaApi *api, MegaTransfer *transfer);
	virtual void QTonTransferFinish(MegaApi* api, MegaTransfer *transfer, MegaError* e);
	virtual void QTonTransferUpdate(MegaApi *api, MegaTransfer *transfer);
	virtual void QTonTransferTemporaryError(MegaApi *api, MegaTransfer *transfer, MegaError* e);
	virtual void QTonUsersUpdate(MegaApi* api, UserList *users);
	virtual void QTonNodesUpdate(MegaApi* api, NodeList *nodes);
	virtual void QTonReloadNeeded(MegaApi* api);

protected:
	MegaListener *listener;
};

#endif // QTMEGALISTENER_H
