#include "QTMegaRequestListener.h"

QTMegaRequestListener::QTMegaRequestListener() : QObject()
{
	cout << "Connection signals" << endl;
	connect(this, SIGNAL(QTonRequestStartSignal(MegaApi *, MegaRequest *)),
			this, SLOT(QTonRequestStart(MegaApi *, MegaRequest *)));
	connect(this, SIGNAL(QTonRequestFinishSignal(MegaApi *, MegaRequest *, MegaError *)),
			this, SLOT(QTonRequestFinish(MegaApi *, MegaRequest *, MegaError *)));
	connect(this, SIGNAL(QTonRequestTemporaryErrorSignal(MegaApi *, MegaRequest *, MegaError *)),
			this, SLOT(QTonRequestTemporaryError(MegaApi *, MegaRequest *, MegaError *)));
}

void QTMegaRequestListener::onRequestStart(MegaApi *api, MegaRequest *request)
{
	emit QTonRequestStartSignal(api, request);
}

void QTMegaRequestListener::onRequestFinish(MegaApi *api, MegaRequest *request, MegaError *e)
{
	cout << "Request finished. Emit" << endl;
	emit QTonRequestFinishSignal(api, request->copy(), e->copy());
}

void QTMegaRequestListener::onRequestTemporaryError(MegaApi *api, MegaRequest *request, MegaError *e)
{
	emit QTonRequestTemporaryErrorSignal(api, request, e->copy());
}

void QTMegaRequestListener::QTonRequestStart(MegaApi *, MegaRequest *request)
{
}

void QTMegaRequestListener::QTonRequestFinish(MegaApi *, MegaRequest *request, MegaError *e)
{
	delete request;
	delete e;
}

void QTMegaRequestListener::QTonRequestTemporaryError(MegaApi *, MegaRequest *request, MegaError *e)
{
	delete e;
}
