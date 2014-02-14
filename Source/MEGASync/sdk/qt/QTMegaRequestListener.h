#ifndef QTMEGAREQUESTLISTENER_H
#define QTMEGAREQUESTLISTENER_H

#include <QObject>
#include "megaapi.h"

class QTMegaRequestListener : public QObject, public MegaRequestListener
{
	Q_OBJECT

public:
    QTMegaRequestListener(MegaApi *megaApi, MegaRequestListener *listener = NULL);
    virtual ~QTMegaRequestListener();

	//Request callbacks
	virtual void onRequestStart(MegaApi* api, MegaRequest *request);
	virtual void onRequestFinish(MegaApi* api, MegaRequest *request, MegaError* e);
	virtual void onRequestTemporaryError(MegaApi *api, MegaRequest *request, MegaError* e);

signals:
	void QTonRequestStartSignal(MegaApi* api, MegaRequest *request);
	void QTonRequestFinishSignal(MegaApi* api, MegaRequest *request, MegaError* e);
	void QTonRequestTemporaryErrorSignal(MegaApi *api, MegaRequest *request, MegaError* e);

public slots:
	virtual void QTonRequestStart(MegaApi* api, MegaRequest *request);
	virtual void QTonRequestFinish(MegaApi* api, MegaRequest *request, MegaError* e);
	virtual void QTonRequestTemporaryError(MegaApi *api, MegaRequest *request, MegaError* e);

protected:
	MegaRequestListener *listener;
    MegaApi *megaApi;
};

#endif // QTMEGAREQUESTLISTENER_H
