#ifndef LOCALCLEANSCHEDULER_H
#define LOCALCLEANSCHEDULER_H

#include <QDialog>
#include "QMegaMessageBox.h"

namespace Ui {
class LocalCleanScheduler;
}

class LocalCleanScheduler : public QDialog
{
    Q_OBJECT

public:
    explicit LocalCleanScheduler(QWidget *parent = 0);
    bool daysLimit();
    int daysLimitValue();
    void setDaysLimit(bool value);
    void setDaysLimitValue(int limit);

    ~LocalCleanScheduler();

private:
    Ui::LocalCleanScheduler *ui;

protected:
    void changeEvent(QEvent *event);

private slots:
    void on_cRemoveFilesOlderThan_clicked();
    void on_bOK_clicked();
    void on_bCancel_clicked();
};

#endif // LOCALCLEANSCHEDULER_H
