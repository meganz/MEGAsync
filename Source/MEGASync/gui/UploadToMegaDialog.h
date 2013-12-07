#ifndef UPLOADTOMEGADIALOG_H
#define UPLOADTOMEGADIALOG_H

#include <QDialog>
#include "sdk/megaapi.h"
#include "sdk/qt/QTMegaRequestListener.h"

namespace Ui {
class UploadToMegaDialog;
}

class UploadToMegaDialog : public QDialog, public MegaRequestListener
{
	Q_OBJECT

public:
	explicit UploadToMegaDialog(MegaApi *megaApi, QWidget *parent = 0);
	~UploadToMegaDialog();
	handle getSelectedHandle();
	bool isDefaultFolder();

	virtual void onRequestFinish(MegaApi *api, MegaRequest *request, MegaError *e);

private slots:
	void on_bChange_clicked();

	void on_buttonBox_accepted();

private:
	Ui::UploadToMegaDialog *ui;
	MegaApi *megaApi;
	handle selectedHandle;

	QTMegaRequestListener *delegateListener;
};

#endif // UPLOADTOMEGADIALOG_H
