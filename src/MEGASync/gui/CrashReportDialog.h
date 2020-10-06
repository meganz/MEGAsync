#ifndef CRASHREPORTDIALOG_H
#define CRASHREPORTDIALOG_H

#include <QDialog>
#include "HighDpiResize.h"

namespace Ui {
class CrashReportDialog;
}

class CrashReportDialog : public QDialog
{
    Q_OBJECT

public:
    explicit CrashReportDialog(QString crash, QWidget *parent = 0);
    QString getUserMessage();
    bool sendLogs();
    ~CrashReportDialog();

private:
    Ui::CrashReportDialog *ui;
    HighDpiResize highDpiResize;
};

#endif // CRASHREPORTDIALOG_H
