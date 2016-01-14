#ifndef STREAMINGFROMMEGADIALOG_H
#define STREAMINGFROMMEGADIALOG_H

#include <QDialog>
#include <QPointer>
#include <QClipboard>
#include <QMessageBox>

#include "megaapi.h"
#include "control/LinkProcessor.h"

namespace Ui {
class StreamingFromMegaDialog;
}

class StreamingFromMegaDialog : public QDialog, public mega::MegaRequestListener
{
    Q_OBJECT

public:
    enum linkstatus {LOADING, CORRECT, WARNING, FAILED};

    explicit StreamingFromMegaDialog(mega::MegaApi *megaApi, QWidget *parent = 0);
    ~StreamingFromMegaDialog();

    virtual void onRequestFinish(mega::MegaApi* api, mega::MegaRequest *request, mega::MegaError* e);

protected:
    void changeEvent(QEvent * event);
    void closeEvent(QCloseEvent * event);

private slots:
    void on_bFromCloud_clicked();
    void on_bFromPublicLink_clicked();
    void on_bCopyLink_clicked();
    void on_bClose_clicked();
    void on_bOpenDefault_clicked();
    void on_bOpenOther_clicked();
private:
    Ui::StreamingFromMegaDialog *ui;
    mega::MegaApi *megaApi;
    mega::QTMegaRequestListener *delegateListener;
    mega::MegaNode *selectedMegaNode;
    QString streamURL;

    bool generateStreamURL();
    void updateFileInfo(QString fileName, linkstatus status);
    void onLinkInfoAvailable();
    void openStreamWithApp(QString app);
};

#endif // STREAMINGFROMMEGADIALOG_H
