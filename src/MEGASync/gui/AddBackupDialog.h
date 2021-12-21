#ifndef ADDBACKUPDIALOG_H
#define ADDBACKUPDIALOG_H

#include <QDialog>
#include <QDir>

namespace Ui {
class AddBackupDialog;
}

class AddBackupDialog : public QDialog
{
    Q_OBJECT

public:
    explicit AddBackupDialog(QWidget *parent = nullptr);
    ~AddBackupDialog();

    void setMyBackupsFolder(QString folder);
    QDir getSelectedFolder();

public slots:
    void on_changeButton_clicked();

private:
    Ui::AddBackupDialog *mUi;
    QDir mSelectedFolder;
};

#endif // ADDBACKUPDIALOG_H
