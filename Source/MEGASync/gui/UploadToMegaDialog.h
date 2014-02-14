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
    void initialize();
    mega::handle getSelectedHandle();
	bool isDefaultFolder();

	virtual void onRequestFinish(MegaApi *api, MegaRequest *request, MegaError *e);

private slots:
	void on_bChange_clicked();
    void on_bOK_clicked();

protected:
    void changeEvent(QEvent * event);

private:
	Ui::UploadToMegaDialog *ui;
	MegaApi *megaApi;
    mega::handle selectedHandle;
	QTMegaRequestListener *delegateListener;
};

#endif // UPLOADTOMEGADIALOG_H
