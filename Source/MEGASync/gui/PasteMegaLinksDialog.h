#ifndef PASTEMEGALINKSDIALOG_H
#define PASTEMEGALINKSDIALOG_H

#include <QDialog>

namespace Ui {
class PasteMegaLinksDialog;
}

class PasteMegaLinksDialog : public QDialog
{
	Q_OBJECT

public:
	explicit PasteMegaLinksDialog(QWidget *parent = 0);
	~PasteMegaLinksDialog();
	QStringList getLinks();

private slots:
	void on_bSubmit_clicked();

private:
	Ui::PasteMegaLinksDialog *ui;
	QStringList links;

	QStringList extractLinks(QString text);
	QString checkLink(QString link);
};

#endif // PASTEMEGALINKSDIALOG_H
