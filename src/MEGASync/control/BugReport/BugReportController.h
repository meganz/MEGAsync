#ifndef BUGREPORTSENDER_H
#define BUGREPORTSENDER_H

#include "QTMegaTransferListener.h"

#include <BugReportData.h>
#include <MegaSyncLogger.h>
#include <QObject>

class BugReportController: public QObject, public mega::MegaTransferListener
{
    Q_OBJECT

public:
    BugReportController(MegaSyncLogger& logger);
    ~BugReportController();

    const BugReportData& getData() const
    {
        return mData;
    }

    void submitReport();

    void attachLogToReport(bool state);
    void setReportDescription(const QString& text);
    void setReportTitle(const QString& text);

    void prepareForCancellation();
    void cancel();
    void resume();

    // needs to be public because it is used within the RequestListenerManager class
    void onRequestFinish(mega::MegaRequest* request, mega::MegaError* e);

signals:
    void reportStarted();
    void reportUpdated(int progress);
    void reportUploadFinished();
    void reportFinished();
    void reportFailed();

private slots:
    void onTransferStart(mega::MegaApi*, mega::MegaTransfer* transfer);
    void onTransferUpdate(mega::MegaApi* api, mega::MegaTransfer* transfer);
    void onTransferFinish(mega::MegaApi* api, mega::MegaTransfer* transfer, mega::MegaError* error);
    void onTransferTemporaryError(mega::MegaApi* api,
                                  mega::MegaTransfer* transfer,
                                  mega::MegaError* e);

    void onReadyForReporting();

private:
    void onReportFailed();
    void createSupportTicket();

    BugReportData mData;

    mega::MegaApi* mMegaApi;
    std::unique_ptr<mega::QTMegaTransferListener> mDelegateTransferListener;

    MegaSyncLogger& mLogger;
};

#endif // BUGREPORTSENDER_H
