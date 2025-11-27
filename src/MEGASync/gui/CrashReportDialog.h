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
    explicit CrashReportDialog(QWidget* parent = 0);
    QString getUserMessage();
    bool sendLogs();
    ~CrashReportDialog();

protected:
    bool event(QEvent* event) override;

private:
    Ui::CrashReportDialog *ui;
};

#endif // CRASHREPORTDIALOG_H
