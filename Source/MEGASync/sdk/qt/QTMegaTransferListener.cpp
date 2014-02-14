#include "QTMegaTransferListener.h"

QTMegaTransferListener::QTMegaTransferListener(MegaApi *megaApi, MegaTransferListener *listener) : QObject()
{
    this->megaApi = megaApi;
	this->listener = listener;
	connect(this, SIGNAL(QTonTransferStartSignal(MegaApi *, MegaTransfer *)),
			this, SLOT(QTonTransferStart(MegaApi *, MegaTransfer *)));
	connect(this, SIGNAL(QTonTransferFinishSignal(MegaApi *, MegaTransfer *, MegaError *)),
			this, SLOT(QTonTransferFinish(MegaApi *, MegaTransfer *, MegaError *)));
	connect(this, SIGNAL(QTonTransferUpdateSignal(MegaApi *, MegaTransfer *)),
			this, SLOT(QTonTransferUpdate(MegaApi *, MegaTransfer *)));
	connect(this, SIGNAL(QTonTransferTemporaryErrorSignal(MegaApi *, MegaTransfer *, MegaError *)),
            this, SLOT(QTonTransferTemporaryError(MegaApi *, MegaTransfer *, MegaError *)));
}

QTMegaTransferListener::~QTMegaTransferListener()
{
    this->listener = NULL;
    megaApi->removeTransferListener(this);
}

void QTMegaTransferListener::onTransferStart(MegaApi *api, MegaTransfer *transfer)
{
	emit QTonTransferStartSignal(api, transfer->copy());
}

void QTMegaTransferListener::onTransferFinish(MegaApi *api, MegaTransfer *transfer, MegaError *e)
{
	emit QTonTransferFinishSignal(api, transfer->copy(), e->copy());
}

void QTMegaTransferListener::onTransferUpdate(MegaApi *api, MegaTransfer *transfer)
{
	emit QTonTransferUpdateSignal(api, transfer->copy());
}

void QTMegaTransferListener::onTransferTemporaryError(MegaApi *api, MegaTransfer *transfer, MegaError *e)
{
    emit QTonTransferTemporaryErrorSignal(api, transfer->copy(), e->copy());
}

void QTMegaTransferListener::QTonTransferStart(MegaApi *api, MegaTransfer *transfer)
{
	if(listener) listener->onTransferStart(api, transfer);
	delete transfer;
}

void QTMegaTransferListener::QTonTransferFinish(MegaApi *api, MegaTransfer *transfer, MegaError *e)
{
	if(listener) listener->onTransferFinish(api, transfer, e);
	delete transfer;
	delete e;
}

void QTMegaTransferListener::QTonTransferUpdate(MegaApi *api, MegaTransfer *transfer)
{
	if(listener) listener->onTransferUpdate(api, transfer);
	delete transfer;
}

void QTMegaTransferListener::QTonTransferTemporaryError(MegaApi *api, MegaTransfer *transfer, MegaError *e)
{
	if(listener) listener->onTransferTemporaryError(api, transfer, e);
	delete transfer;
	delete e;
}
