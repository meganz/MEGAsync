#ifndef ADDEXCLUSIONDIALOG_H
#define ADDEXCLUSIONDIALOG_H

#include "syncs/control/MegaIgnoreManager.h"
#include <QDialog>

namespace Ui {
class AddExclusionDialog;
}

class AddExclusionDialog : public QDialog
{
    Q_OBJECT

public:
    explicit AddExclusionDialog(const QString& syncLocalFolder,QWidget *parent = 0);
    ~AddExclusionDialog();
    QString textValue();

private slots:
    void on_bOk_clicked();
    void on_bChoose_clicked();

#ifndef __APPLE__
    void on_bChooseFile_clicked();
#endif

protected:
    void changeEvent(QEvent * event);

private:
    void setTextToExclusionItem(const QString& path);
    void addItem(const QStringList& selection);

    Ui::AddExclusionDialog *ui;
    QString mSyncLocalFolder;
};

#endif // ADDEXCLUSIONDIALOG_H
