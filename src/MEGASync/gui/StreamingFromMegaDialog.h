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

class StreamingFromMegaDialog : public QDialog, public mega::MegaTransferListener
{
    Q_OBJECT

    static const uint8_t NODE_ID;

public:
    enum class LinkStatus {LOADING=0, CORRECT, WARNING, TRANSFER_OVER_QUOTA};
    enum class LastStreamingSelection {NOT_SELECTED=0, FROM_LOCAL_NODE, FROM_PUBLIC_NODE};

    explicit StreamingFromMegaDialog(mega::MegaApi *megaApi,mega::MegaApi* megaApiFolders, QWidget *parent = 0);
    ~StreamingFromMegaDialog();

    void onTransferTemporaryError(mega::MegaApi *api, mega::MegaTransfer *transfer, mega::MegaError* e) override;

public slots:
    void updateStreamingState();

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

    //Link processor signals for public links streaming
    void onLinkInfoAvailable();

private:
    std::unique_ptr<Ui::StreamingFromMegaDialog> ui;
    std::unique_ptr<LinkProcessor> mLinkProcessor;
    mega::MegaApi *megaApi;
    mega::MegaApi* mMegaApiFolders;
    std::unique_ptr<mega::QTMegaTransferListener> delegateTransferListener;
    std::shared_ptr<mega::MegaNode> mSelectedMegaNode;
    QString streamURL;
    HighDpiResize highDpiResize;
    LastStreamingSelection lastStreamSelection;
    QString mPublicLink;

    bool generateStreamURL();
    void updateFileInfo(QString fileName, LinkStatus status);
    void openStreamWithApp(QString app);
    void showStreamingError();
    void hideStreamingError();
    void updateFileInfoFromNode(mega::MegaNode* node);
    void requestPublicNodeInfo();
    void requestNodeToLinkProcessor();
};

#endif // STREAMINGFROMMEGADIALOG_H
