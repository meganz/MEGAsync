#ifndef BINDFOLDERDIALOG_H
#define BINDFOLDERDIALOG_H

#include "HighDpiResize.h"
#include <megaapi.h>

#include <QDialog>


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

    mega::MegaHandle getMegaFolder();
    void setMegaFolder(mega::MegaHandle handle);
    QString getLocalFolder();
    QString getSyncName();

    QString getMegaPath() const;

private slots:
    void on_bOK_clicked();

protected:
    void changeEvent(QEvent * event);

private:
    Ui::BindFolderDialog *ui;
    MegaApplication *app;
    QString syncName;
    QString megaPath;

    QStringList localFolders;

    HighDpiResize highDpiResize;
};

#endif // BINDFOLDERDIALOG_H
