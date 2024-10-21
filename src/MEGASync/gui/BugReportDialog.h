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

    virtual void onTransferStart(mega::MegaApi* api, mega::MegaTransfer* transfer);
    virtual void onTransferUpdate(mega::MegaApi* api, mega::MegaTransfer* transfer);
    virtual void onTransferFinish(mega::MegaApi* api,
                                  mega::MegaTransfer* transfer,
                                  mega::MegaError* error);
    virtual void onTransferTemporaryError(mega::MegaApi* api,
                                          mega::MegaTransfer* transfer,
                                          mega::MegaError* e);

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
