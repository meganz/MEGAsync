#ifndef PASTEMEGALINKSDIALOG_H
#define PASTEMEGALINKSDIALOG_H

#include <QDialog>
#include "HighDpiResize.h"

namespace Ui {
class PasteMegaLinksDialog;
}

class PasteMegaLinksDialog : public QDialog
{
    Q_OBJECT

public:
    static const int FILE_LINK_SIZE = 70;
    static const int NEW_FILE_LINK_SIZE = 73;

    static const int FOLDER_LINK_SIZE = 50;
    static const int NEW_FOLDER_LINK_SIZE = 54;

    static const int FOLDER_LINK_WITH_SUBFOLDER_SIZE = 59;
    static const int NEW_FOLDER_LINK_WITH_SUBFOLDER_SIZE = 70;

    explicit PasteMegaLinksDialog(QWidget *parent = 0);
    ~PasteMegaLinksDialog();
    QStringList getLinks();

private slots:
    void on_bSubmit_clicked();

protected:
    void changeEvent(QEvent * event);

private:
    Ui::PasteMegaLinksDialog *ui;
    QStringList links;

    QStringList extractLinks(QString text);
    QString checkLink(QString link);

    HighDpiResize highDpiResize;
};

#endif // PASTEMEGALINKSDIALOG_H
