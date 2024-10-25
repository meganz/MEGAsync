#ifndef BUGREPORTDIALOG_H
#define BUGREPORTDIALOG_H

#include "MegaSyncLogger.h"
#include "ProgressIndicatorDialog.h"

#include <QDialog>
#include <QPointer>

class QProgressDialog;
class BugReportController;

namespace Ui {
class BugReportDialog;
}

class BugReportDialog: public QDialog
{
    Q_OBJECT

public:
    explicit BugReportDialog(QWidget *parent, MegaSyncLogger& logger);
    ~BugReportDialog();

    struct DefaultInfo
    {
        bool sendLogsChecked = true;
        bool sendLogsMandatory = false;
        QString title;
        QString description;
    };

    void setDefaultInfo(const DefaultInfo& info);

private slots:
    void onReportStarted();
    void resetProgressDialog();
    void onReportFinished();
    void onReportFailed();

    void onReportUpdated(int value);

private:
    void closeProgressDialog();

    Ui::BugReportDialog* ui;
    QPointer<QProgressDialog> mSendProgress;
    std::unique_ptr<BugReportController> mController;

    const static int mMaxDescriptionLength = 3000;

protected:
private slots:
    void onSubmitClicked();
    void onCancelClicked();
    void cancelSendReport();
    void onDescriptionChanged();
    void onTitleChanged();
    void on_teDescribeBug_textChanged();
};

#endif // BUGREPORTDIALOG_H
