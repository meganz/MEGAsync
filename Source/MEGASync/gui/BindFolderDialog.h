#ifndef BINDFOLDERDIALOG_H
#define BINDFOLDERDIALOG_H

#include <QDialog>

#include "FolderBinder.h"

namespace Ui {
class BindFolderDialog;
}

class BindFolderDialog : public QDialog
{
    Q_OBJECT

public:
    explicit BindFolderDialog(QWidget *parent = 0);
    ~BindFolderDialog();

    long long getMegaFolder();
    QString getLocalFolder();

private:
    Ui::BindFolderDialog *ui;
};

#endif // BINDFOLDERDIALOG_H
