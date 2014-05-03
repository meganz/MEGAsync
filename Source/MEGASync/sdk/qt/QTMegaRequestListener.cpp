#include "QTMegaRequestListener.h"

QTMegaRequestListener::QTMegaRequestListener(MegaApi *megaApi, MegaRequestListener *listener) : QObject()
{
    this->megaApi = megaApi;
	this->listener = listener;
    connect(this, SIGNAL(QTonRequestStartSignal(MegaApi *, MegaRequest *)),
            this, SLOT(QTonRequestStart(MegaApi *, MegaRequest *)));
    connect(this, SIGNAL(QTonRequestFinishSignal(MegaApi *, MegaRequest *, MegaError *)),
            this, SLOT(QTonRequestFinish(MegaApi *, MegaRequest *, MegaError *)));
    connect(this, SIGNAL(QTonRequestUpdateSignal(MegaApi *, MegaRequest *)),
            this, SLOT(QTonRequestUpdate(MegaApi *, MegaRequest *)));
    connect(this, SIGNAL(QTonRequestTemporaryErrorSignal(MegaApi *, MegaRequest *, MegaError *)),
            this, SLOT(QTonRequestTemporaryError(MegaApi *, MegaRequest *, MegaError *)));
}

QTMegaRequestListener::~QTMegaRequestListener()
{
    this->listener = NULL;
    megaApi->removeRequestListener(this);
}

//Parameters are copied because when the signal is processed the request could be finished
void QTMegaRequestListener::onRequestStart(MegaApi *api, MegaRequest *request)
{
	emit QTonRequestStartSignal(api, request->copy());
}

void QTMegaRequestListener::onRequestFinish(MegaApi *api, MegaRequest *request, MegaError *e)
{
    emit QTonRequestFinishSignal(api, request->copy(), e->copy());
}

void QTMegaRequestListener::onRequestUpdate(MegaApi *api, MegaRequest *request)
{
    emit QTonRequestUpdateSignal(api, request->copy());
}

void QTMegaRequestListener::onRequestTemporaryError(MegaApi *api, MegaRequest *request, MegaError *e)
{
	emit QTonRequestTemporaryErrorSignal(api, request->copy(), e->copy());
}

void QTMegaRequestListener::QTonRequestStart(MegaApi *api, MegaRequest *request)
{
	if(listener) listener->onRequestStart(api, request);
	delete request;
}

void QTMegaRequestListener::QTonRequestFinish(MegaApi *api, MegaRequest *request, MegaError *e)
{
	if(listener) listener->onRequestFinish(api, request, e);
	delete request;
    delete e;
}

void QTMegaRequestListener::QTonRequestUpdate(MegaApi *api, MegaRequest *request)
{
    if(listener) listener->onRequestUpdate(api, request);
    delete request;
}

void QTMegaRequestListener::QTonRequestTemporaryError(MegaApi *api, MegaRequest *request, MegaError *e)
{
	if(listener) listener->onRequestTemporaryError(api, request, e);
	delete request;
    delete e;
}
