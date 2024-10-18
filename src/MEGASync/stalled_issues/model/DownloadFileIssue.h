#ifndef DOWNLOADFILEISSUEFACTORY_H
#define DOWNLOADFILEISSUEFACTORY_H

#include <StalledIssue.h>
#include <TransferItem.h>

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

    void solveIssue();
    static void solveIssues();
    static void addIssueToSolve(const StalledIssueVariant& issueToFix);

private slots:
    void onRetryableSyncTransferRetried(mega::MegaHandle handle);
    void onRetriedSyncTransferFinished(mega::MegaHandle handle, TransferData::TransferState state);

private:
    static QList<mega::MegaHandle> mIssuesToRetry;
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
