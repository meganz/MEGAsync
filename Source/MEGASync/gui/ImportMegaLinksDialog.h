#ifndef IMPORTMEGALINKSDIALOG_H
#define IMPORTMEGALINKSDIALOG_H

#include <QDialog>
#include <QStringList>
#include "sdk/megaapi.h"
#include "utils/LinkProcessor.h"

namespace Ui {
class ImportMegaLinksDialog;
}

class ImportMegaLinksDialog : public QDialog, public MegaRequestListener
{
	Q_OBJECT

public:
	explicit ImportMegaLinksDialog(MegaApi *megaApi, LinkProcessor *linkProcessor, QWidget *parent = 0);
	~ImportMegaLinksDialog();

	bool shouldImport();
	bool shouldDownload();
	QString getImportPath();
	QString getDownloadPath();

private slots:
	void on_cDownload_clicked();
	void on_cImport_clicked();
	void on_bLocalFolder_clicked();
	void on_bMegaFolder_clicked();

public slots:
	void onLinkInfoAvailable(int id);
	void onLinkInfoRequestFinish();

private:
	Ui::ImportMegaLinksDialog *ui;
	MegaApi *megaApi;
	LinkProcessor *linkProcessor;
	bool finished;
};

#endif // IMPORTMEGALINKSDIALOG_H
