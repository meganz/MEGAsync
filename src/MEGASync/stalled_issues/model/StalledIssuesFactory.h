#ifndef STALLEDISSUEFACTORY_H
#define STALLEDISSUEFACTORY_H

#include <StalledIssue.h>
#include <MultiStepIssueSolver.h>

class MoveOrRenameCannotOccurFactory;

class ReceivedStalledIssues
{
public:
    ReceivedStalledIssues() = default;
    ~ReceivedStalledIssues() = default;

    bool isEmpty() const
    {
        return mAutoSolvedStalledIssues.isEmpty() && mActiveStalledIssues.isEmpty() &&
               mFailedAutoSolvedStalledIssues.isEmpty();
    }

    int size() const
    {
        return mAutoSolvedStalledIssues.size() + mActiveStalledIssues.size() +
               mFailedAutoSolvedStalledIssues.size();
    }

    void clear()
    {
        mActiveStalledIssues.clear();
        mAutoSolvedStalledIssues.clear();
        mFailedAutoSolvedStalledIssues.clear();
    }

    StalledIssuesVariantList& autoSolvedStalledIssues()
    {
        return mAutoSolvedStalledIssues;
    }

    StalledIssuesVariantList& activeStalledIssues()
    {
        return mActiveStalledIssues;
    }

    StalledIssuesVariantList& failedAutoSolvedStalledIssues()
    {
        return mFailedAutoSolvedStalledIssues;
    }

private:
    friend class StalledIssuesCreator;

    StalledIssuesVariantList mActiveStalledIssues;
    StalledIssuesVariantList mAutoSolvedStalledIssues;
    StalledIssuesVariantList mFailedAutoSolvedStalledIssues;
};

Q_DECLARE_METATYPE(ReceivedStalledIssues)

class StalledIssuesFactory
{
public:
    StalledIssuesFactory();
    virtual ~StalledIssuesFactory() = default;

    virtual std::shared_ptr<StalledIssue> createIssue(MultiStepIssueSolverBase* solver, const mega::MegaSyncStall* stall) = 0;
    virtual void clear(){}
};

enum class UpdateType
{
    NONE,
    UI,
    EVENT,
    AUTO_SOLVE
};

Q_DECLARE_METATYPE(UpdateType)

class StalledIssuesCreator : public QObject
{
        Q_OBJECT
public:
    struct IssuesCount
    {
        int totalIssues = 0;
        int currentIssueBeingSolved = 0;
        int issuesFixed = 0;
        int issuesFailed = 0;

        bool isEmpty(){return totalIssues + currentIssueBeingSolved + issuesFailed + issuesFixed == 0;}
    };

    StalledIssuesCreator();

    void createIssues(mega::MegaSyncStallList* issues,
        UpdateType updateType);

    bool multiStepIssueSolveActive() const;
    void addMultiStepIssueSolver(MultiStepIssueSolverBase* issue);

    ReceivedStalledIssues getStalledIssues() const;

signals:
    void solvingIssues(IssuesCount count);
    void solvingIssuesFinished(IssuesCount count);

protected:
    void clear();

    ReceivedStalledIssues mStalledIssues;

private:
    QPointer<MultiStepIssueSolverBase> getMultiStepIssueSolverByStall(const mega::MegaSyncStall* stall);

    QMultiMap<mega::MegaSyncStall::SyncStallReason,
        MultiStepIssueSolverBase*> mMultiStepIssueSolversByReason;
    std::shared_ptr<MoveOrRenameCannotOccurFactory> mMoveOrRenameCannotOccurFactory;
};

Q_DECLARE_METATYPE(StalledIssuesCreator::IssuesCount)

#endif // STALLEDISSUEFACTORY_H
