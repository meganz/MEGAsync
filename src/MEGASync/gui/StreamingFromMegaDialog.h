#ifndef STREAMINGFROMMEGADIALOG_H
#define STREAMINGFROMMEGADIALOG_H

#include <QDialog>
#include <QPointer>
#include <QClipboard>
#include <QMessageBox>

#include "megaapi.h"
#include "control/LinkProcessor.h"
#include "HighDpiResize.h"
#include "QTMegaTransferListener.h"
#include <memory>

namespace Ui {
class StreamingFromMegaDialog;
}

class StreamingFromMegaDialog : public QDialog, public mega::MegaRequestListener, public mega::MegaTransferListener
{
    Q_OBJECT

public:
    enum linkstatus {LOADING, CORRECT, WARNING, FAILED};

    explicit StreamingFromMegaDialog(mega::MegaApi *megaApi, QWidget *parent = 0);
    ~StreamingFromMegaDialog();

    void onRequestFinish(mega::MegaApi* api, mega::MegaRequest *request, mega::MegaError* e) override;
    void onTransferTemporaryError(mega::MegaApi *api, mega::MegaTransfer *transfer, mega::MegaError* e) override;

protected:
    void changeEvent(QEvent * event) override;
    void closeEvent(QCloseEvent * event) override;

private slots:
    void on_bFromCloud_clicked();
    void on_bFromPublicLink_clicked();
    void on_bCopyLink_clicked();
    void on_bClose_clicked();
    void on_bOpenDefault_clicked();
    void on_bOpenOther_clicked();
private:
    std::unique_ptr<Ui::StreamingFromMegaDialog> ui;
    mega::MegaApi *megaApi;
    std::unique_ptr<mega::QTMegaRequestListener> delegateListener;
    std::unique_ptr<mega::QTMegaTransferListener> delegateTransferListener;
    std::unique_ptr<mega::MegaNode> selectedMegaNode;
    QString streamURL;
    HighDpiResize highDpiResize;

    bool generateStreamURL();
    void updateFileInfo(QString fileName, linkstatus status);
    void onLinkInfoAvailable();
    void openStreamWithApp(QString app);
    void showStreamingError();
    void hideStreamingError();
};

#endif // STREAMINGFROMMEGADIALOG_H
