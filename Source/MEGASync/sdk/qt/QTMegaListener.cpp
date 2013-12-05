#include "QTMegaListener.h"

QTMegaListener::QTMegaListener(MegaListener *listener)
{
	this->listener = listener;
	connect(this, SIGNAL(QTonRequestStartSignal(MegaApi *, MegaRequest *)),
			this, SLOT(QTonRequestStart(MegaApi *, MegaRequest *)));
	connect(this, SIGNAL(QTonRequestFinishSignal(MegaApi *, MegaRequest *, MegaError *)),
			this, SLOT(QTonRequestFinish(MegaApi *, MegaRequest *, MegaError *)));
	connect(this, SIGNAL(QTonRequestTemporaryErrorSignal(MegaApi *, MegaRequest *, MegaError *)),
			this, SLOT(QTonRequestTemporaryError(MegaApi *, MegaRequest *, MegaError *)));

	connect(this, SIGNAL(QTonTransferStartSignal(MegaApi *, MegaTransfer *)),
			this, SLOT(QTonTransferStart(MegaApi *, MegaTransfer *)));
	connect(this, SIGNAL(QTonTransferFinishSignal(MegaApi *, MegaTransfer *, MegaError *)),
			this, SLOT(QTonTransferFinish(MegaApi *, MegaTransfer *, MegaError *)));
	connect(this, SIGNAL(QTonTransferUpdateSignal(MegaApi *, MegaTransfer *)),
			this, SLOT(QTonTransferUpdate(MegaApi *, MegaTransfer *)));
	connect(this, SIGNAL(QTonTransferTemporaryErrorSignal(MegaApi *, MegaTransfer *, MegaError *)),
			this, SLOT(QTonTransferTemporaryError(MegaApi *, MegaTransfer *, MegaError *)));

	connect(this, SIGNAL(QTonUsersUpdateSignal(MegaApi *, UserList *)),
			this, SLOT(QTonUsersUpdate(MegaApi *, UserList *)),
			Qt::BlockingQueuedConnection);
	connect(this, SIGNAL(QTonNodesUpdateSignal(MegaApi *, NodeList *)),
			this, SLOT(QTonNodesUpdate(MegaApi *, NodeList *)),
			Qt::BlockingQueuedConnection);
	connect(this, SIGNAL(QTonReloadNeededSignal(MegaApi *)),
			this, SLOT(QTonReloadNeeded(MegaApi *)));

}

void QTMegaListener::onRequestStart(MegaApi *api, MegaRequest *request)
{
	emit QTonRequestStartSignal(api, request->copy());
}

void QTMegaListener::onRequestFinish(MegaApi *api, MegaRequest *request, MegaError *e)
{
	emit QTonRequestFinishSignal(api, request->copy(), e->copy());
}

void QTMegaListener::onRequestTemporaryError(MegaApi *api, MegaRequest *request, MegaError *e)
{
	emit QTonRequestTemporaryErrorSignal(api, request->copy(), e->copy());
}

void QTMegaListener::onTransferStart(MegaApi *api, MegaTransfer *transfer)
{
	emit QTonTransferStartSignal(api, transfer->copy());
}

void QTMegaListener::onTransferFinish(MegaApi *api, MegaTransfer *transfer, MegaError *e)
{
	emit QTonTransferFinishSignal(api, transfer->copy(), e->copy());
}

void QTMegaListener::onTransferUpdate(MegaApi *api, MegaTransfer *transfer)
{
	emit QTonTransferUpdateSignal(api, transfer->copy());
}

void QTMegaListener::onTransferTemporaryError(MegaApi *api, MegaTransfer *transfer, MegaError *e)
{
	emit QTonTransferTemporaryErrorSignal(api, transfer->copy(), e);
}

void QTMegaListener::onUsersUpdate(MegaApi *api, UserList *users)
{
	emit QTonUsersUpdateSignal(api, users);
}

void QTMegaListener::onNodesUpdate(MegaApi *api, NodeList *nodes)
{
	emit QTonNodesUpdateSignal(api, nodes);
}

void QTMegaListener::onReloadNeeded(MegaApi *api)
{
	emit QTonReloadNeeded(api);
}

void QTMegaListener::QTonRequestStart(MegaApi *api, MegaRequest *request)
{
	if(listener) listener->onRequestStart(api, request);
	delete request;
}

void QTMegaListener::QTonRequestFinish(MegaApi *api, MegaRequest *request, MegaError *e)
{
	if(listener) listener->onRequestFinish(api, request, e);
	delete request;
	delete e;
}

void QTMegaListener::QTonRequestTemporaryError(MegaApi *api, MegaRequest *request, MegaError *e)
{
	if(listener) listener->onRequestTemporaryError(api, request, e);
	delete request;
	delete e;
}

void QTMegaListener::QTonTransferStart(MegaApi *api, MegaTransfer *transfer)
{
	if(listener) listener->onTransferStart(api, transfer);
	delete transfer;
}

void QTMegaListener::QTonTransferFinish(MegaApi *api, MegaTransfer *transfer, MegaError *e)
{
	if(listener) listener->onTransferFinish(api, transfer, e);
	delete transfer;
	delete e;
}

void QTMegaListener::QTonTransferUpdate(MegaApi *api, MegaTransfer *transfer)
{
	if(listener) listener->onTransferUpdate(api, transfer);
	delete transfer;
}

void QTMegaListener::QTonTransferTemporaryError(MegaApi *api, MegaTransfer *transfer, MegaError *e)
{
	if(listener) listener->onTransferTemporaryError(api, transfer, e);
	delete transfer;
	delete e;
}

void QTMegaListener::QTonUsersUpdate(MegaApi *api, UserList *users)
{
	if(listener) listener->onUsersUpdate(api, users);
}

void QTMegaListener::QTonNodesUpdate(MegaApi *api, NodeList *nodes)
{
	if(listener) listener->onNodesUpdate(api, nodes);
}

void QTMegaListener::QTonReloadNeeded(MegaApi *api)
{
	if(listener) listener->onReloadNeeded(api);
}
