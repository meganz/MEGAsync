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
    static const int FILE_LINK_SIZE = 54;
    static const int NEW_FILE_LINK_SIZE = 57;

    static const int FOLDER_LINK_SIZE = 34;
    static const int NEW_FOLDER_LINK_SIZE = 38;

    static const int FOLDER_LINK_WITH_SUBFOLDER_SIZE = 43;
    static const int NEW_FOLDER_LINK_WITH_SUBFOLDER_SIZE = 54;

    static const int FOLDER_LINK_WITH_FILE_SIZE = 43;
    static const int NEW_FOLDER_LINK_WITH_FILE_SIZE = 52;


    QString base64regExp = QString::fromUtf8("A-Za-z0-9_-");

    QRegExp rxHeaderFile = QRegExp(QString::fromUtf8("^#![%1]{8}![%1]{43}").arg(base64regExp));
    QRegExp rxHeaderFileNew = QRegExp(QString::fromUtf8("^file\\/[%1]{8}#[%1]{43}").arg(base64regExp));

    QRegExp rxHeaderFolder = QRegExp(QString::fromUtf8("^#F![%1]{8}![%1]{22}").arg(base64regExp));
    QRegExp rxHeaderFolderNew = QRegExp(QString::fromUtf8("^folder\\/[%1]{8}#[%1]{22}").arg(base64regExp));

    QRegExp rxHeaderFolderSubfolder = QRegExp(QString::fromUtf8("^#F![%1]{8}![%1]{22}![%1]{8}").arg(base64regExp));
    QRegExp rxHeaderFolderSubfolderNew = QRegExp(QString::fromUtf8("^folder\\/[%1]{8}#[%1]{22}\\/folder\\/[%1]{8}").arg(base64regExp));

    QRegExp rxHeaderFolderFile = QRegExp(QString::fromUtf8("^#F![%1]{8}![%1]{22}\\?[%1]{8}").arg(base64regExp));
    QRegExp rxHeaderFolderFileNew = QRegExp(QString::fromUtf8("^folder\\/[%1]{8}#[%1]{22}\\/file\\/[%1]{8}").arg(base64regExp));

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
};

#endif // PASTEMEGALINKSDIALOG_H
