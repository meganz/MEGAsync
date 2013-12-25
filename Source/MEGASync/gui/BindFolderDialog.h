#ifndef BINDFOLDERDIALOG_H
#define BINDFOLDERDIALOG_H

#include <QDialog>

#include "FolderBinder.h"

class MegaApplication;

namespace Ui {
class BindFolderDialog;
}

class BindFolderDialog : public QDialog
{
    Q_OBJECT

public:
    explicit BindFolderDialog(MegaApplication *app, QWidget *parent = 0);
    ~BindFolderDialog();

    long long getMegaFolder();
    QString getLocalFolder();
    QString getSyncName();

private slots:
    void on_buttonBox_accepted();

private:
    Ui::BindFolderDialog *ui;
    MegaApplication *app;
    QString syncName;
};

#endif // BINDFOLDERDIALOG_H
