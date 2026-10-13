#ifndef STREAMINGFROMMEGADIALOG_H
#define STREAMINGFROMMEGADIALOG_H

#include "LinkProcessor.h"
#include "megaapi.h"
#include "TransferQuota.h"

#include <QClipboard>
#include <QDialog>
#include <QMessageBox>
#include <QPointer>

#include <memory>

namespace Ui {
class StreamingFromMegaDialog;
}

class StreamingFromMegaDialog: public QDialog
{
    Q_OBJECT

    static const uint8_t NODE_ID;

public:
    enum class LinkStatus {LOADING=0, CORRECT, WARNING, TRANSFER_OVER_QUOTA};
    enum class LastStreamingSelection {NOT_SELECTED=0, FROM_LOCAL_NODE, FROM_PUBLIC_NODE};

    explicit StreamingFromMegaDialog(mega::MegaApi *megaApi,mega::MegaApi* megaApiFolders, QWidget *parent = 0);
    ~StreamingFromMegaDialog();

public slots:
    void updateStreamingState();
    void onTransferQuotaStateChanged(QuotaState state);

protected:
    bool event(QEvent* event) override;
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
    std::shared_ptr<mega::MegaNode> mSelectedMegaNode;
    QString streamURL;
    LastStreamingSelection lastStreamSelection;
    QString mPublicLink;
    bool mStreamStarted = false;

    bool generateStreamURL();
    void onStreamStarted();
    void showTransferOverQuota();
    void updateFileInfo(QString fileName, LinkStatus status);
    void openStreamWithApp(QString app);
    void showStreamingError();
    void hideStreamingError();
    void updateFileInfoFromNode(mega::MegaNode* node);
    void requestPublicNodeInfo();
    void requestNodeToLinkProcessor();
    void showErrorMessage(const QString& message);
    void setErrorBannerText();
};

#endif // STREAMINGFROMMEGADIALOG_H
