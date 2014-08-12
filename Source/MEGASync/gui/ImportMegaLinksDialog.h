#ifndef IMPORTMEGALINKSDIALOG_H
#define IMPORTMEGALINKSDIALOG_H

#include <QDialog>
#include <QStringList>
#include "sdk/megaapi.h"
#include "control/LinkProcessor.h"
#include "control/Preferences.h"

namespace Ui {
class ImportMegaLinksDialog;
}

class ImportMegaLinksDialog : public QDialog
{
	Q_OBJECT

public:
    explicit ImportMegaLinksDialog(mega::MegaApi *megaApi, Preferences *preferences, LinkProcessor *linkProcessor, QWidget *parent = 0);
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
	void onLinkStateChanged(int id, int state);

protected:
    void changeEvent(QEvent * event);

private:
	Ui::ImportMegaLinksDialog *ui;
    mega::MegaApi *megaApi;
	LinkProcessor *linkProcessor;
	bool finished;
};

#endif // IMPORTMEGALINKSDIALOG_H
