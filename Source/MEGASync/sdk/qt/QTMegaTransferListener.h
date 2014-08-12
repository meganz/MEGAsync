#ifndef QTMEGATRANSFERLISTENER_H
#define QTMEGATRANSFERLISTENER_H

#include <QObject>
#include <sdk/megaapi.h>

namespace mega
{
class QTMegaTransferListener : public QObject, public MegaTransferListener
{
	Q_OBJECT

public:
    QTMegaTransferListener(MegaApi *megaApi,MegaTransferListener *listener);
    virtual ~QTMegaTransferListener();

public:
	virtual void onTransferStart(MegaApi *api, MegaTransfer *transfer);
	virtual void onTransferFinish(MegaApi* api, MegaTransfer *transfer, MegaError* e);
	virtual void onTransferUpdate(MegaApi *api, MegaTransfer *transfer);
	virtual void onTransferTemporaryError(MegaApi *api, MegaTransfer *transfer, MegaError* e);

signals:
	void QTonTransferStartSignal(MegaApi *api, MegaTransfer *transfer);
	void QTonTransferFinishSignal(MegaApi* api, MegaTransfer *transfer, MegaError* e);
	void QTonTransferUpdateSignal(MegaApi *api, MegaTransfer *transfer);
	void QTonTransferTemporaryErrorSignal(MegaApi *api, MegaTransfer *transfer, MegaError* e);

public slots:
	virtual void QTonTransferStart(MegaApi *api, MegaTransfer *transfer);
	virtual void QTonTransferFinish(MegaApi* api, MegaTransfer *transfer, MegaError* e);
	virtual void QTonTransferUpdate(MegaApi *api, MegaTransfer *transfer);
	virtual void QTonTransferTemporaryError(MegaApi *api, MegaTransfer *transfer, MegaError* e);

protected:
    MegaApi *megaApi;
	MegaTransferListener *listener;
};
}

#endif // QTMEGATRANSFERLISTENER_H
