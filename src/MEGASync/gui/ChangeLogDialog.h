#ifndef CHANGELOGDIALOG_H
#define CHANGELOGDIALOG_H

#include "control/Preferences.h"
#include <QDialog>

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

    void setChangeLogNotes(QString notes);

protected:
    void changeEvent(QEvent *event);

private slots:
    void on_bTerms_clicked();
    void on_bPolicy_clicked();
    void on_bAck_clicked();
};

#endif // CHANGELOGDIALOG_H
