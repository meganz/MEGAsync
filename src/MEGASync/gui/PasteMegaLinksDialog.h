#ifndef PASTEMEGALINKSDIALOG_H
#define PASTEMEGALINKSDIALOG_H

#include <QDialog>

#define FILE_LINK_SIZE 70
#define FOLDER_LINK_SIZE 50

namespace Ui {
class PasteMegaLinksDialog;
}

class PasteMegaLinksDialog : public QDialog
{
    Q_OBJECT

public:

    typedef enum {
           ONLY_FILE_LINKS = 0,
           ONLY_FOLDER_LINKS = 1
    } ExtractionMode;

    explicit PasteMegaLinksDialog(QWidget *parent = 0);
    ~PasteMegaLinksDialog();
    QStringList getFileLinks();
    QStringList getFolderLinks();

private slots:
    void on_bSubmit_clicked();

protected:
    void changeEvent(QEvent * event);

private:
    Ui::PasteMegaLinksDialog *ui;
    QStringList fileLinks;
    QStringList folderLinks;

    QStringList extractLinks(QString text, int extraction = 0);
    QString checkLink(QString link);
};

#endif // PASTEMEGALINKSDIALOG_H
