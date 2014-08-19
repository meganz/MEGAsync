#ifndef UPLOADTOMEGADIALOG_H
#define UPLOADTOMEGADIALOG_H

#include <QDialog>
#include "megaapi.h"
#include "sdk/qt/QTMegaRequestListener.h"


namespace Ui {
class UploadToMegaDialog;
}

class UploadToMegaDialog : public QDialog, public mega::MegaRequestListener
{
	Q_OBJECT

public:
    explicit UploadToMegaDialog(mega::MegaApi *megaApi, QWidget *parent = 0);
	~UploadToMegaDialog();
    mega::MegaHandle getSelectedHandle();
	bool isDefaultFolder();

    virtual void onRequestFinish(mega::MegaApi *api, mega::MegaRequest *request, mega::MegaError *e);

private slots:
	void on_bChange_clicked();
    void on_bOK_clicked();

protected:
    void changeEvent(QEvent * event);

private:
	Ui::UploadToMegaDialog *ui;
    mega::MegaApi *megaApi;
    mega::MegaHandle selectedHandle;
    mega::QTMegaRequestListener *delegateListener;
};

#endif // UPLOADTOMEGADIALOG_H
