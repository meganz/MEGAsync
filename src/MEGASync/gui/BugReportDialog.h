#ifndef BUGREPORTDIALOG_H
#define BUGREPORTDIALOG_H

// clang-format off
#include "MegaSyncLogger.h"
#include "ProgressIndicatorDialog.h"

#include <QPointer>
#include <QDialog>
// clang-format on

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

private slots:
    void onReportStarted();
    void onReportFinished();
    void onReportFailed();
    void onReportUploadedFinished();

    void onReportUpdated(int value);

private:
    void closeProgressDialog();
    void openProgressDialog();

    Ui::BugReportDialog* ui;
    QPointer<ProgressIndicatorDialog> mProgressIndicatorDialog;
    std::unique_ptr<BugReportController> mController;

    const static int mMaxDescriptionLength = 3000;

private slots:
    void onSubmitClicked();
    void onCancelClicked();
    void cancelSendReport();
    void onDescriptionChanged();
    void onTitleChanged();
    void onDescribeBugTextChanged();
};

#endif // BUGREPORTDIALOG_H
