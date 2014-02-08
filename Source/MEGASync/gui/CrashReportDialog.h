#ifndef CRASHREPORTDIALOG_H
#define CRASHREPORTDIALOG_H

#include <QDialog>

namespace Ui {
class CrashReportDialog;
}

class CrashReportDialog : public QDialog
{
    Q_OBJECT

public:
    explicit CrashReportDialog(QString crash, QWidget *parent = 0);
    QString getUserMessage();
    ~CrashReportDialog();

private:
    Ui::CrashReportDialog *ui;
};

#endif // CRASHREPORTDIALOG_H
