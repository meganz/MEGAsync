#ifndef STALLEDISSUESDIALOG_H
#define STALLEDISSUESDIALOG_H

#include "MegaDelegateHoverManager.h"

#include <QDialog>

namespace Ui {
class StalledIssuesDialog;
}

class StalledIssuesDialog : public QDialog
{
    Q_OBJECT

public:
    explicit StalledIssuesDialog(QWidget *parent = nullptr);
    ~StalledIssuesDialog();

private slots:
    void on_doneButton_clicked();
    void on_updateButton_clicked();

private:
    Ui::StalledIssuesDialog *ui;
    MegaDelegateHoverManager mViewHoverManager;
};

#endif // STALLEDISSUESDIALOG_H
