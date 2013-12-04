#ifndef UPLOADTOMEGADIALOG_H
#define UPLOADTOMEGADIALOG_H

#include <QDialog>

namespace Ui {
class UploadToMegaDialog;
}

class UploadToMegaDialog : public QDialog
{
	Q_OBJECT

public:
	explicit UploadToMegaDialog(QWidget *parent = 0);
	~UploadToMegaDialog();

private:
	Ui::UploadToMegaDialog *ui;
};

#endif // UPLOADTOMEGADIALOG_H
