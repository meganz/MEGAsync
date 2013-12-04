#ifndef LINKPROCESSOR_H
#define LINKPROCESSOR_H

#include <QObject>
#include <QStringList>
#include "sdk/megaapi.h"
#include "sdk/qt/QTMegaRequestListener.h"

class LinkProcessor: public QTMegaRequestListener
{
	Q_OBJECT

public:
	LinkProcessor(MegaApi *megaApi, QStringList linkList);
	QStringList getLinkList();
	QString getLink(int id);
	bool isSelected(int id);
	bool getError(int id);
	Node *getNode(int id);
	int size();

	void requestLinkInfo();
	void importLinks(QString megaPath);
	void importLinks(Node *node);

	void downloadLinks(QString localPath);
	void setSelected(int linkId, bool selected);

	int numSuccessfullImports();
	int numFailedImports();

protected:
	MegaApi *megaApi;
	QStringList linkList;
	QList<bool> linkSelected;
	QList<Node*> linkNode;
	QList<int> linkError;
	int currentIndex;
	int remainingNodes;
	int importSuccess;
	int importFailed;

signals:
	void onLinkInfoAvailable(int i);
	void onLinkInfoRequestFinish();
	void onLinkImportFinish();

public slots:
	virtual void QTonRequestFinish(MegaApi* api, MegaRequest *request, MegaError* e);
};

#endif // LINKPROCESSOR_H
