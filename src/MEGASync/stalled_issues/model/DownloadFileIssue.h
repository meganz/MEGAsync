#ifndef DOWNLOADFILEISSUEFACTORY_H
#define DOWNLOADFILEISSUEFACTORY_H

#include "StalledIssue.h"
#include "TransferTrack.h"

#include <optional>

class DownloadFileIssueFactory
{
public:
    DownloadFileIssueFactory() = default;
    ~DownloadFileIssueFactory() = default;

    static StalledIssueSPtr createAndFillIssue(const mega::MegaSyncStall* stall);
};

class DownloadIssue: public StalledIssue
{
public:
    enum IssueType
    {
        UNKNOWN,
        FINGERPRINT,
        NODEBLOCKED
    };

public:
    DownloadIssue(std::optional<IssueType> type, const mega::MegaSyncStall* stall);
    std::optional<IssueType> getType() const;

protected:
    DownloadIssue(const mega::MegaSyncStall* stall);

    std::optional<IssueType> mType;
};

class BugReportController;

class UnknownDownloadIssue: public DownloadIssue
{
public:
    enum SolveOptionSelected
    {
        RETRY,
        SEND_FEEDBACK
    };

    UnknownDownloadIssue(const mega::MegaSyncStall* stall);
    ~UnknownDownloadIssue() = default;

    void fillIssue(const mega::MegaSyncStall* stall) override;
    bool canBeRetried() const;
    bool checkForExternalChanges() override;

    void solveIssueByRetry();

    void sendFeedback();
    bool isSendingFeedback() const;

private slots:
    void onTrackedTransferStarted(TransferItem transfer);
    void onTrackedTransferFinished(TransferItem transfer);

    void onReportStarted();
    void onReportFinished();
    void onReportFailed();

private:
    void connectTrack();

    std::shared_ptr<TransferTrack> mTrack;
    std::shared_ptr<BugReportController> mReportController;
};

class InvalidFingerprintDownloadIssue: public DownloadIssue
{
public:
    InvalidFingerprintDownloadIssue(const mega::MegaSyncStall* stall);
    ~InvalidFingerprintDownloadIssue() = default;

    static void solveIssues();
    static void addIssueToSolve(const StalledIssueVariant& issueToFix);

private:
    static QList<StalledIssueVariant> mFingerprintIssuesToFix;
};

#endif // DOWNLOADFILEISSUEFACTORY_H
