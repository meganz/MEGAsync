#ifndef CHANGELOGDIALOG_H
#define CHANGELOGDIALOG_H

#include "control/Preferences.h"
#include <QDialog>
#include "HighDpiResize.h"

namespace Ui {
class ChangeLogDialog;
}

class ChangeLogDialog : public QDialog
{
    Q_OBJECT

public:
    explicit ChangeLogDialog(QString version, QString SDKversion, QString changeLog, QWidget *parent = 0);
    ~ChangeLogDialog();

private:
    Ui::ChangeLogDialog *ui;
    HighDpiResize highDpiResize;

    void setChangeLogNotes(QString notes);

protected:
    void changeEvent(QEvent *event);
    void tweakStrings();

private slots:
    void on_bTerms_clicked();
    void on_bPolicy_clicked();
    void on_bAck_clicked();
    void on_bGDPR_clicked();
};

#endif // CHANGELOGDIALOG_H
