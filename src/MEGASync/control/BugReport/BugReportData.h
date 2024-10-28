#ifndef BUGREPORTDATA_H
#define BUGREPORTDATA_H

#include <megaapi.h>
#include <optional>
#include <QFileInfo>
#include <QString>

class BugReportData
{
public:
    enum class STATUS
    {
        STOPPED,
        PREPARING_LOG,
        LOG_FAILED,
        LOG_READY,
        UPLOADING_LOG,
        LOG_UPLOAD_CANCELLED,
        LOG_UPLOAD_FAILED,
        LOG_UPLOADED_SUCCESSFULLY,
        SUBMITTING_REPORT,
        REPORT_SUBMIT_FAILED,
        REPORT_SUBMITTED
    };

    static const int MAXIMUM_PERMIL_VALUE;

    const QString& getTitle() const
    {
        return mReportTitle;
    }

    const QString& getDescription() const
    {
        return mReportDescription;
    }

    const QString& getReportFileName() const
    {
        return mReportFileName;
    }

    const QString& getReportPath() const
    {
        return mReportPath;
    }

    int getCurrentTransfer() const
    {
        return mCurrentTransfer;
    }

    long long getTotalBytes() const
    {
        return mTotalBytes;
    }

    long long getTransferredBytes() const
    {
        return mTransferredBytes;
    }

    int getLastPermil() const
    {
        return mLastpermil;
    }

    STATUS getStatus() const
    {
        return mStatus;
    }

    bool isCancelled() const
    {
        return mCancelled;
    }

    bool getAttachLog() const
    {
        return mAttachLog;
    }

    int getTransferError() const
    {
        return mTransferError;
    }

    int getRequestError() const
    {
        return mRequestError;
    }

    bool getHadGlobalPause() const
    {
        return mHadGlobalPause;
    }

    void resetStates()
    {
        mCurrentTransfer = 0;
        mTotalBytes = 0;
        mTransferredBytes = 0;
        mLastpermil = -3;
        mStatus = STATUS::STOPPED;
        mCancelled = false;
        mTransferError = mega::MegaError::API_OK;
        mRequestError = mega::MegaError::API_OK;
    }

private:
    friend class BugReportController;

    QString mReportTitle;
    QString mReportDescription;
    QString mReportFileName;
    QString mReportPath;

    int mCurrentTransfer = 0;
    long long mTotalBytes = 0;
    long long mTransferredBytes = 0;
    int mLastpermil = -3;

    bool mCancelled = false;
    STATUS mStatus = STATUS::STOPPED;

    bool mAttachLog = false;

    int mTransferError = mega::MegaError::API_OK;
    int mRequestError = mega::MegaError::API_OK;

    bool mHadGlobalPause = false;
};

#endif // BUGREPORTDATA_H
