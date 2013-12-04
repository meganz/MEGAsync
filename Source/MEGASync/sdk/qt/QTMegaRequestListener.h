#ifndef QTMEGAREQUESTLISTENER_H
#define QTMEGAREQUESTLISTENER_H

#include <QObject>
#include <sdk/megaapi.h>

class QTMegaRequestListener : public QObject, public MegaRequestListener
{
	Q_OBJECT

public:
	explicit QTMegaRequestListener();

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
};

#endif // QTMEGAREQUESTLISTENER_H
