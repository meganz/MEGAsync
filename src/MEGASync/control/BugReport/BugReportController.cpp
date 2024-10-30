#include "BugReportController.h"

#include <MegaApplication.h>
#include <Preferences.h>
#include <QTMegaApiManager.h>
#include <RequestListenerManager.h>
#include <Utilities.h>

const int BugReportData::MAXIMUM_PERMIL_VALUE = 1010;

BugReportController::BugReportController(MegaSyncLogger& logger):
    mLogger(logger)
{
    mMegaApi = ((MegaApplication*)qApp)->getMegaApi();
    mDelegateTransferListener = std::make_unique<mega::QTMegaTransferListener>(mMegaApi, this);

    connect(&mLogger,
            &MegaSyncLogger::logReadyForReporting,
            this,
            &BugReportController::onReadyForReporting);
}

BugReportController::~BugReportController()
{
    if (mega::QTMegaApiManager::isMegaApiValid(mMegaApi))
    {
        // Just in case the dialog is closed from an exit action
        cancel();
    }
}

void BugReportController::submitReport()
{
    mData.resetStates();

    if (mLogger.prepareForReporting())
    {
        mData.mStatus = BugReportData::STATUS::PREPARING_LOG;

        emit reportStarted();
    }
    else
    {
        mData.mStatus = BugReportData::STATUS::LOG_FAILED;

        onReportFailed();
    }
}

void BugReportController::attachLogToReport(bool state)
{
    mData.mAttachLog = state;
}

void BugReportController::setReportDescription(const QString& text)
{
    mData.mReportDescription = text;
}

void BugReportController::setReportTitle(const QString& text)
{
    mData.mReportTitle = text;
}

void BugReportController::onTransferStart(mega::MegaApi*, mega::MegaTransfer* transfer)
{
    mData.mStatus = BugReportData::STATUS::UPLOADING_LOG;

    if (!mData.mCurrentTransfer)
    {
        mData.mCurrentTransfer = transfer->getTag();
    }

    mData.mTotalBytes = transfer->getTotalBytes();
}

void BugReportController::onTransferUpdate(mega::MegaApi*, mega::MegaTransfer* transfer)
{
    if (transfer->getState() == mega::MegaTransfer::STATE_ACTIVE)
    {
        mData.mTransferredBytes = transfer->getTransferredBytes();
        int permil = (mData.mTotalBytes > 0) ?
                         static_cast<int>((1000 * mData.mTransferredBytes) / mData.mTotalBytes) :
                         0;
        if (permil > mData.mLastpermil)
        {
            mData.mLastpermil = permil;
            emit reportUpdated(mData.mLastpermil);
        }
    }
}

void BugReportController::onTransferFinish(mega::MegaApi*,
                                           mega::MegaTransfer* transfer,
                                           mega::MegaError* error)
{
    if (mData.mHadGlobalPause)
    {
        MegaSyncApp->getTransfersModel()->setGlobalPause(true);
    }

    if (transfer->getState() == mega::MegaTransfer::STATE_CANCELLED)
    {
        mData.mStatus = BugReportData::STATUS::LOG_UPLOAD_CANCELLED;

        // Only if the transfer has been cancelled from the TM
        if (!mData.mCancelled)
        {
            onReportFailed();
        }
    }
    else
    {
        mData.mTransferError = error->getErrorCode();

        if (mData.mTransferError == mega::MegaError::API_OK)
        {
            mData.mStatus = BugReportData::STATUS::LOG_UPLOADED_SUCCESSFULLY;
            emit reportUploadFinished();
            createSupportTicket();
        }
        else
        {
            mData.mStatus = BugReportData::STATUS::LOG_UPLOAD_FAILED;
            onReportFailed();
        }
    }
}

void BugReportController::onTransferTemporaryError(mega::MegaApi*,
                                                   mega::MegaTransfer*,
                                                   mega::MegaError* e)
{
    mega::MegaApi::log(
        mega::MegaApi::LOG_LEVEL_ERROR,
        QString::fromUtf8("Temporary error at report dialog: %1")
            .arg(QString::fromUtf8(
                mega::MegaError::getErrorString(e->getErrorCode(), mega::MegaError::API_EC_UPLOAD)))
            .toUtf8()
            .constData());
}

void BugReportController::onRequestFinish(mega::MegaRequest* request, mega::MegaError* e)
{
    switch (request->getType())
    {
        case mega::MegaRequest::TYPE_SUPPORT_TICKET:
        {
            mData.mRequestError = e->getErrorCode();

            if (mData.mRequestError == mega::MegaError::API_OK)
            {
                mData.mStatus = BugReportData::STATUS::REPORT_SUBMITTED;
                emit reportFinished();
            }
            else
            {
                mData.mStatus = BugReportData::STATUS::REPORT_SUBMIT_FAILED;
                onReportFailed();
            }

            break;
        }
        default:
            break;
    }
}

void BugReportController::prepareForCancellation()
{
    mData.mCancelled = true;

    if (mData.mStatus == BugReportData::STATUS::UPLOADING_LOG && mData.mCurrentTransfer)
    {
        MegaSyncApp->getTransfersModel()->pauseResumeTransferByTag(mData.mCurrentTransfer, true);
    }
}

void BugReportController::cancel()
{
    if (mData.mStatus == BugReportData::STATUS::UPLOADING_LOG && mData.mCurrentTransfer)
    {
        mMegaApi->cancelTransferByTag(mData.mCurrentTransfer);

        if (mData.mHadGlobalPause)
        {
            MegaSyncApp->getTransfersModel()->setGlobalPause(true);
        }
    }
}

void BugReportController::resume()
{
    mData.mCancelled = false;

    if (mData.getStatus() == BugReportData::STATUS::LOG_UPLOADED_SUCCESSFULLY)
    {
        createSupportTicket();
    }
    else
    {
        emit reportUpdated(mData.getLastPermil());

        // If there is a transfer, the logger finished the zipped log
        if (mData.getStatus() == BugReportData::STATUS::UPLOADING_LOG && mData.mCurrentTransfer > 0)
        {
            MegaSyncApp->getTransfersModel()->pauseResumeTransferByTag(mData.mCurrentTransfer,
                                                                       false);
        }
        // The logger finished the zipped log but the user cancelled it before uploading it
        else if (mData.getStatus() == BugReportData::STATUS::LOG_READY)
        {
            onReadyForReporting();
        }
        else
        {
            mData.mStatus = BugReportData::STATUS::REPORT_SUBMIT_FAILED;
            onReportFailed();
        }
    }
}

void BugReportController::onReadyForReporting()
{
    mData.mStatus = BugReportData::STATUS::LOG_READY;

    // Check if the user stopped the process
    if (!mData.mCancelled)
    {
        mData.mReportFileName.clear();

        // If send log file is enabled
        if (mData.getAttachLog())
        {
            mData.mReportPath = Utilities::joinLogZipFiles(mMegaApi);
            if (mData.mReportPath.isNull())
            {
                mLogger.resumeAfterReporting();
                onReportFailed();
            }
            else
            {
                QFileInfo joinLogsFile{mData.mReportPath};
                mData.mReportFileName = joinLogsFile.fileName();

                if (Preferences::instance()->getGlobalPaused())
                {
                    mData.mHadGlobalPause = true;
                    MegaSyncApp->getTransfersModel()->setGlobalPause(false);
                }

                mMegaApi->startUploadForSupport(
                    QDir::toNativeSeparators(mData.mReportPath).toUtf8().constData(),
                    true,
                    mDelegateTransferListener.get());
            }
        }
        else
        {
            // Create support ticket
            createSupportTicket();
        }
    }
}

void BugReportController::onReportFailed()
{
    emit reportFailed();
}

void BugReportController::createSupportTicket()
{
    emit reportUpdated(BugReportData::MAXIMUM_PERMIL_VALUE);

    QString report =
        QString::fromUtf8("Version string: %1   Version code: %2.%3   User-Agent: %4\n")
            .arg(Preferences::VERSION_STRING)
            .arg(Preferences::VERSION_CODE)
            .arg(Preferences::BUILD_ID)
            .arg(QString::fromUtf8(mMegaApi->getUserAgent()));

    report.append(QString::fromUtf8("Report filename: %1")
                      .arg(mData.mReportFileName.isEmpty() ? QString::fromUtf8("Not sent") :
                                                             mData.mReportFileName)
                      .append(QString::fromUtf8("\n")));
    report.append(
        QString::fromUtf8("Title: %1").arg(mData.mReportTitle.append(QString::fromUtf8("\n"))));
    report.append(QString::fromUtf8("Description: %1")
                      .arg(mData.mReportDescription.append(QString::fromUtf8("\n"))));

    auto listener = RequestListenerManager::instance().registerAndGetFinishListener(this, true);
    mMegaApi->createSupportTicket(report.toUtf8().constData(), 6, listener.get());

    mData.mStatus = BugReportData::STATUS::SUBMITTING_REPORT;
}
