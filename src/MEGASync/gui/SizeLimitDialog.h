#ifndef SIZELIMITDIALOG_H
#define SIZELIMITDIALOG_H

#include <QDialog>
#include <QObject>
#include <QMessageBox>
#include "Preferences.h"
#include "HighDpiResize.h"

namespace Ui {
class SizeLimitDialog;
}

class SizeLimitDialog : public QDialog
{
    Q_OBJECT

public:
    explicit SizeLimitDialog(QWidget *parent = 0);
    bool upperSizeLimit();
    bool lowerSizeLimit();
    void setUpperSizeLimit(bool value);
    void setLowerSizeLimit(bool value);
    void setUpperSizeLimitValue(long long limit);
    void setLowerSizeLimitValue(long long limit);
    long long upperSizeLimitValue();
    long long lowerSizeLimitValue();
    void setUpperSizeLimitUnit(int unit);
    void setLowerSizeLimitUnit(int unit);
    int upperSizeLimitUnit();
    int lowerSizeLimitUnit();

    ~SizeLimitDialog();

private:
    Ui::SizeLimitDialog *ui;
    HighDpiResize highDpiResize;

protected:
    void changeEvent(QEvent *event);

private slots:
    void on_cExcludeUpperThan_clicked();
    void on_cExcludeLowerThan_clicked();
    void on_bOK_clicked();
    void on_bCancel_clicked();
};

#endif // SIZELIMITDIALOG_H
