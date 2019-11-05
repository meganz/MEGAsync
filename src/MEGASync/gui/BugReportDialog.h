#ifndef BUGREPORTDIALOG_H
#define BUGREPORTDIALOG_H

#include <QDialog>
#include "megaapi.h"
#include "QTMegaTransferListener.h"
#include "MegaApplication.h"
#include <QProgressDialog>

namespace Ui {
class BugReportDialog;
}

class BugReportDialog : public QDialog, public mega::MegaTransferListener
{
    Q_OBJECT

public:
    explicit BugReportDialog(QWidget *parent = 0);
    ~BugReportDialog();

    virtual void onTransferStart(mega::MegaApi *api, mega::MegaTransfer *transfer);
    virtual void onTransferUpdate(mega::MegaApi *api, mega::MegaTransfer *transfer);
    virtual void onTransferFinish(mega::MegaApi* api, mega::MegaTransfer *transfer, mega::MegaError* error);
    virtual void onTransferTemporaryError(mega::MegaApi *api, mega::MegaTransfer *transfer, mega::MegaError* e);

private:
    Ui::BugReportDialog *ui;
    int currentTransfer;
    QProgressDialog sendProgress{this};

    long long totalBytes;
    long long transferredBytes;

    bool warningShown;

protected:
    mega::MegaApi *megaApi;
    mega::QTMegaTransferListener *delegateListener;

private slots:
    void on_bSubmit_clicked();
    void on_bCancel_clicked();
    void cancelSendReport();
    void onDescriptionChanged();
};

#endif // BUGREPORTDIALOG_H
